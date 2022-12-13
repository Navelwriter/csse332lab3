#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>

#define simple_assert(message, test) \
  do                                 \
  {                                  \
    if (!(test))                     \
      return message;                \
  } while (0)
#define TEST_PASSED NULL
#define DATA_SIZE 100
#define INITIAL_VALUE 77
#define MAX_TESTS 10

/* test_funcs is an array of function pointers that store all of the test we
 * would like to run. */
char *(*test_funcs[MAX_TESTS])();

/* total number of tests registered */
static int num_tests = 0;

/* shared data that all the tests will use. */
static int data[DATA_SIZE][DATA_SIZE];

/**
 * Add a test to our list of test functions that we would like to run.
 *
 * @param test_func A pointer to the function that we would like to run.
 */
void add_test(char *(*test_func)())
{
  if (num_tests == MAX_TESTS)
  {
    printf("exceeded max possible tests");
    exit(1);
  }
  test_funcs[num_tests] = test_func;
  num_tests++;
}

/**
 * This setup function should run before each test
 */
void setup(void)
{
  printf("starting setup\n");
  for (int i = 0; i < DATA_SIZE; i++)
  {
    for (int j = 0; j < DATA_SIZE; j++)
    {
      data[i][j] = INITIAL_VALUE;
    }
  }

  /* imagine this function does a lot of other complicated setup that takes a
   * long time. */
  usleep(1000000);
}

// signal handler
void sig_handler(int signo)
{
  if (signo == SIGALRM)
  {
    exit(2);
  }
}

/**
 * Run all the test in the test suite.
 */
void run_all_tests()
{
  int fd[2];
  pipe(fd);
  // Create an array of child process IDs, one for each test
  pid_t pids[num_tests];
  signal(SIGALRM, sig_handler);
  setup();
  // Fork a child process for each test
  for (int i = 0; i < num_tests; i++)
  {
    pid_t pid = fork();
    if (!pid)
    {
      alarm(3);
      // checkf or error signal
      char *message = test_funcs[i]();
      // check if message WIFEXITED to see if test crashed

      if (message == TEST_PASSED)
      {
        write(fd[1], message, 1024);
        close(fd[1]);
        exit(0);
      }

      else
      {
        write(fd[1], message, 1024);
        close(fd[1]);
        exit(1);
      }
    }

    else
      // This is the parent process
      pids[i] = pid;
  }

  // Wait for all child processes to complete
  for (int i = 0; i < num_tests; i++)
  {
    int status;
    pid_t pid1 = waitpid(pids[i], &status, 0);
    char buffer[1024];
    int bytes_read = read(fd[0], buffer, 1024);
    // clear buffer
    buffer[bytes_read] = '\0';
    if (WIFEXITED(status) == 0)
    {
      printf("Test failed: Crashed\n");
    }
    else if (WEXITSTATUS(status) == 0)
      printf("Test passed\n");
    else if (WEXITSTATUS(status) == 2)
      printf("Test failed: Time out\n");
    else
    {
      printf("Test failed %s\n", buffer);
    }
    close(fd[0]);
  }
}

char *test1()
{
  printf("starting test 1\n");
  for (int i = 0; i < DATA_SIZE; i++)
  {
    for (int j = 0; j < DATA_SIZE; j++)
    {
      simple_assert("test 1 data not initialized properly",
                    data[i][j] == INITIAL_VALUE);
    }
  }

  for (int i = 0; i < DATA_SIZE; i++)
  {
    for (int j = 0; j < DATA_SIZE; j++)
    {
      data[i][j] = 1;
    }
  }

  for (int i = 0; i < DATA_SIZE; i++)
  {
    for (int j = 0; j < DATA_SIZE; j++)
    {
      simple_assert("test 1 data not set properly", data[i][j] == 1);
    }
  }
  printf("ending test 1\n");
  return TEST_PASSED;
}

char *test2()
{
  printf("starting test 2\n");
  for (int i = 0; i < DATA_SIZE; i++)
  {
    for (int j = 0; j < DATA_SIZE; j++)
    {
      simple_assert("test 2 data not initialized properly",
                    data[i][j] == INITIAL_VALUE);
    }
  }

  for (int i = 0; i < DATA_SIZE; i++)
  {
    for (int j = 0; j < DATA_SIZE; j++)
    {
      data[i][j] = 2;
    }
  }

  for (int i = 0; i < DATA_SIZE; i++)
  {
    for (int j = 0; j < DATA_SIZE; j++)
    {
      simple_assert("test 2 data not set properly", data[i][j] == 2);
    }
  }

  printf("ending test 2\n");
  return TEST_PASSED;
}

char *test3()
{
  printf("starting test 3\n");

  simple_assert("test 3 always fails", 1 == 2);

  printf("ending test 3\n");
  return TEST_PASSED;
}

char *test4()
{
  printf("starting test 4\n");

  int *val = NULL;
  printf("data at val is %d", *val);

  printf("ending test 4\n");
  return TEST_PASSED;
}

char *test5()
{
  printf("starting test 5\n");

  while (1)
    ;

  printf("ending test 5\n");
  return TEST_PASSED;
}

int main(int argc, char **argv)
{
  add_test(test1);
  add_test(test2);
  add_test(test3);
  add_test(test4);
  add_test(test5);
  run_all_tests();
}
