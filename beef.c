/*
 * beef - calculate events per <n/sec> from an input stream
 *
 * Ben LeMasurier 2k12'
 * beerware.
 */
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <signal.h>
#include <sys/time.h>
#include <pthread.h>

static struct {
  char *progname;
  uintmax_t events;
  int interval;
  struct itimerval itv;
  pthread_mutex_t lock;
} beef;

static void
display(int i)
{
  (void) i;
  signal(SIGALRM, display);
  pthread_mutex_lock(&beef.lock);
  printf("%" PRIuMAX "\n", beef.events);
  beef.events = 0;
  pthread_mutex_unlock(&beef.lock);

  setitimer(ITIMER_REAL, &beef.itv, 0);
}

static void
destroy(int i)
{
  (void) i;
  pthread_mutex_destroy(&beef.lock);

  exit(EXIT_SUCCESS);
}

static void
usage()
{
  printf("%s: usage: %s <interval in seconds>\n", 
      beef.progname, beef.progname);

  pthread_exit(NULL);
  exit(EXIT_SUCCESS);
}

static void *
event_monitor()
{
  int c;
  while((c = fgetc(stdin)) != EOF) {
    if(c == '\n') {
      pthread_mutex_lock(&beef.lock);
      beef.events += 1;
      pthread_mutex_unlock(&beef.lock);
    }
  }
}

int
main(int argc, char *argv[])
{
  pthread_t event_thread;
  struct itimerval itv;
  beef.progname = argv[0];

  if(argc < 2)
    usage();

  beef.events = 0;
  beef.interval = atoi(argv[1]);
  pthread_mutex_init(&beef.lock, NULL);
  beef.itv.it_interval.tv_sec  = 0;
  beef.itv.it_interval.tv_usec = 0;
  beef.itv.it_value.tv_sec = beef.interval;
  beef.itv.it_value.tv_usec = 0;
  setitimer(ITIMER_REAL, &beef.itv, 0);
  signal(SIGALRM, display);
  signal(SIGINT, destroy);

  pthread_create(&event_thread, NULL, event_monitor, NULL);
  while(1) {}

  return EXIT_SUCCESS;
}
