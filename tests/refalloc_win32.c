
#include "../refalloc.h"
#include <process.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <windows.h>

static void test_destructor(void*);
static unsigned __stdcall test_thread(void *);
struct test_box* test_constructor(int j);
static int thread_join(uintptr_t thread_ptr, unsigned int* result);
static char const* thread_join_strerror(int errnum);

struct test_box {
  int j;
};

struct test_box* test_boxes[10] = {
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL,
  NULL
};

int test_repeat_count = 1000000;

struct test_box* test_constructor(int j){
  struct test_box* box =
    refalloc_malloc(sizeof(struct test_box), &test_destructor);
  assert(box != NULL);
  box->j = j;
  fprintf(stderr, "created item %d\n", box->j);
  return box;
}
void test_destructor(void* arg){
  struct test_box* const box = (struct test_box*)arg;
  fprintf(stderr, "destroying item %d\n", box->j);
  box->j = -1;
  return;
}

unsigned __stdcall test_thread(void *arg){
  int const thread_id = *(int*)arg;
  int const repeat_count = test_repeat_count;
  int i;
  int personal_refs[10];
  int faults = 0;
  int overdrops = 0;
  int outstanding = 0;
  fprintf(stderr, "[Thread %i starting]\n",thread_id);
  srand((int)(time(NULL)));
  for (i = 0; i < 10; ++i){
    personal_refs[i] = 0;
  }
  for (i = 0; i < repeat_count; ++i){
    int const step_rand = rand();
    int command = step_rand%2;
    int j = (step_rand/2)%10;
    if (command == 0){
      /* acquire */
      struct test_box* box;
      box = refalloc_acquire(test_boxes[j]);
      if (box == test_boxes[j]){
        personal_refs[j] += 1;
      } else faults += 1;
    } else {
      /* release */
      if (personal_refs[j] > 0){
        refalloc_release(test_boxes[j]);
        personal_refs[j] -= 1;
      } else overdrops += 1;
    }
  }
  for (i = 9; i >= 0; --i){
    while (personal_refs[i] > 0){
      refalloc_release(test_boxes[i]);
      outstanding += 1;
      personal_refs[i] -= 1;
    }
  }
  fprintf(stderr,
      "[Thread %i done: %i faults, %i overdrops, %i outstanding]\n",
      thread_id, faults, overdrops, outstanding);
  return 0;
}

int thread_join(uintptr_t thread_ptr, unsigned int* result) {
  DWORD res = WAIT_TIMEOUT;
  while (res == WAIT_TIMEOUT) {
    res = WaitForSingleObject((HANDLE)thread_ptr, 5000);
  }
  if (result != NULL) {
    DWORD thread_result;
    GetExitCodeThread((HANDLE)thread_ptr, &thread_result);
    *result = (unsigned int)thread_result;
  }
  return res;
}
char const* thread_join_strerror(int errnum) {
  switch (errnum) {
  case WAIT_TIMEOUT:
    return "Timeout";
  case WAIT_ABANDONED:
    return "Abandoned";
  case WAIT_FAILED:
    return "Wait failed";
  case WAIT_IO_COMPLETION:
    return "Input/output completion event triggered";
  case WAIT_OBJECT_0:
    return "Success";
  default:
    return "Generic error";
  }
}

int main(int argc, char **argv){
  int corrupt_count = 0;
  uintptr_t four_threads[4];
  int thread_i_array[4];
  /* make the boxes */{
    int box_j;
    for (box_j = 0; box_j < 10; ++box_j){
      test_boxes[box_j] = test_constructor(box_j);
    }
  }
  /* make the threads */{
    int thread_i;
    for (thread_i = 0; thread_i < 4; ++thread_i){
      unsigned int thread_id;
      thread_i_array[thread_i] = thread_i+1;
      four_threads[thread_i] = _beginthreadex
        ( NULL, 0, test_thread, thread_i_array + thread_i,
          0, &thread_id);
      if (four_threads[thread_i] != 0) continue;
      else {
        fprintf(stderr, "Failed to create thread %i\n\terror win32:%i\n",
            thread_i+1, GetLastError());
        thread_i_array[thread_i] = -1;
        continue;
      }
    }
  }
  /* ... sit and wait ... */
  /* join threads */{
    int thread_i;
    for (thread_i = 0; thread_i < 4; ++thread_i){
      int errnum;
      if (thread_i_array[thread_i] == (thread_i+1)){
        unsigned int result;
        errnum = thread_join(four_threads[thread_i], &result);
        if (errnum == 0) continue;
        else {
          fprintf(stderr, "Failed to join thread %i\n\t%s\n",
              thread_i, thread_join_strerror(errnum));
          continue;
        }
      }
    }
  }
  /* inspect boxes */{
    int box_j;
    for (box_j = 0; box_j < 10; ++box_j){
      if (test_boxes[box_j]->j != box_j){
        fprintf(stderr, "Box %i was corrupted.\n", box_j);
        corrupt_count += 1;
      }
      refalloc_release(test_boxes[box_j]);
      test_boxes[box_j] = NULL;
    }
  }
  return corrupt_count==0?EXIT_SUCCESS:EXIT_FAILURE;
}
