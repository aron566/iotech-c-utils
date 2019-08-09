#include <iot/threadpool.h>
#include <iot/iot.h>
#include <CUnit.h>
#include "scheduler.h"

static int sum_test;
static int infinity_test;
static int sum_work1, sum_work2, sum_work3;
static int counter;
static iot_logger_t *logger = NULL;

static void do_work1 (void *in)
{
  for (int i = 0; i < 10; ++i)
  {
    sum_work1++;
  }
}

static void do_work2 (void *in)
{
  for (int i = 0; i < 20; ++i)
  {
    sum_work2 += i;
  }
}

static void do_work3 (void *in)
{
  for (int i = 0; i < 30; ++i)
  {
    sum_work3 += i;
  }
}

static void do_work4 (void *in)
{
  sum_test++;
}

static void do_work5 (void *in)
{
  infinity_test++;
}

static void do_count (void *in)
{
  counter++;
}

static int suite_init (void)
{
  logger = iot_logger_alloc ("Test", IOT_LOG_WARN);
  iot_logger_start (logger);
  return 0;
}

static int suite_clean (void)
{
  iot_logger_free (logger);
  return 0;
}

static void cunit_scheduler_start (void)
{
  iot_threadpool_t *pool = iot_threadpool_alloc (4, 0, NULL, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (pool, logger);

  CU_ASSERT (iot_threadpool_start (pool))
  CU_ASSERT (iot_scheduler_start (scheduler))

  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_work1, NULL, IOT_MS_TO_NS (500), 0, 0, NULL);
  CU_ASSERT (sched1 != NULL)
  iot_schedule_t *sched2 = iot_schedule_create (scheduler, do_work2, NULL, IOT_SEC_TO_NS (1), 0, 0, NULL);
  CU_ASSERT (sched2 != NULL)
  iot_schedule_t *sched3 = iot_schedule_create (scheduler, do_work3, NULL, IOT_SEC_TO_NS (1), 0, 0, NULL);
  CU_ASSERT (sched3 != NULL)

  CU_ASSERT (iot_schedule_add (scheduler, sched1))
  CU_ASSERT (iot_schedule_add (scheduler, sched2))

  sleep (2);

  CU_ASSERT (sum_work1 > 0)
  CU_ASSERT (sum_work2 > 0)
  CU_ASSERT (sum_work3 == 0)

  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_stop (void)
{
  iot_threadpool_t *pool = iot_threadpool_alloc (4, 0, NULL, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (pool, logger);

  CU_ASSERT (iot_threadpool_start (pool))
  CU_ASSERT (iot_scheduler_start (scheduler))
  sum_test = 0;

  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_work4, NULL, IOT_MS_TO_NS (1), 0, 1, NULL);
  CU_ASSERT (sched1 != NULL)
  CU_ASSERT (iot_schedule_add (scheduler, sched1))

  sleep (2);
  iot_scheduler_stop (scheduler);
  CU_ASSERT (sum_test == 1)

  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_create (void)
{
  iot_threadpool_t *pool = iot_threadpool_alloc (4, 0, NULL, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (pool, logger);

  CU_ASSERT (iot_threadpool_start (pool))
  CU_ASSERT (iot_scheduler_start (scheduler))
  sum_test = 0;

  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_work4, NULL, IOT_MS_TO_NS (250), 0, 1, NULL);
  iot_schedule_t *sched2 = iot_schedule_create (scheduler, do_work5, NULL, IOT_MS_TO_NS (1), 0, 0, NULL);

  CU_ASSERT (sched1 != NULL)
  CU_ASSERT (sched2 != NULL)
  CU_ASSERT (iot_schedule_add (scheduler, sched1))
  CU_ASSERT (iot_schedule_add (scheduler, sched2))
  iot_scheduler_start (scheduler);

  sleep (1);
  iot_scheduler_stop (scheduler);
  CU_ASSERT (sum_test == 1)
  CU_ASSERT (infinity_test > 5)

  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_remove (void)
{
  iot_threadpool_t *pool = iot_threadpool_alloc (4, 0, NULL, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (pool, logger);

  CU_ASSERT (iot_threadpool_start (pool))
  CU_ASSERT (iot_scheduler_start (scheduler))
  sum_test = 0;

  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_work4, NULL, IOT_MS_TO_NS (1), 0, 1, NULL);
  iot_schedule_t *sched2 = iot_schedule_create (scheduler, do_work5, NULL, IOT_MS_TO_NS (1), 0, 0, NULL);
  iot_schedule_t *sched3 = iot_schedule_create (scheduler, do_work5, NULL, IOT_MS_TO_NS (1), 0, 0, NULL);
  iot_schedule_t *sched4 = iot_schedule_create (scheduler, do_work3, NULL, IOT_MS_TO_NS (1), 0, 0, NULL);
  iot_schedule_t *sched5 = iot_schedule_create (scheduler, do_work3, NULL, IOT_MS_TO_NS (1), 0, 0, NULL);
  iot_schedule_t *sched6 = iot_schedule_create (scheduler, do_work3, NULL, IOT_MS_TO_NS (1), 0, 0, NULL);

  CU_ASSERT (sched1 != NULL)
  CU_ASSERT (sched2 != NULL)
  CU_ASSERT (sched3 != NULL)
  CU_ASSERT (sched4 != NULL)
  CU_ASSERT (sched5 != NULL)
  CU_ASSERT (sched6 != NULL)
  CU_ASSERT (iot_schedule_add (scheduler, sched1))
  CU_ASSERT (iot_schedule_add (scheduler, sched2))
  CU_ASSERT (iot_schedule_add (scheduler, sched3))
  CU_ASSERT (iot_schedule_add (scheduler, sched4))
  CU_ASSERT (iot_schedule_add (scheduler, sched5))
  CU_ASSERT (iot_schedule_add (scheduler, sched6))

  sleep (1);

  iot_schedule_remove (scheduler, sched2);
  iot_schedule_remove (scheduler, sched3);
  CU_ASSERT (sum_test == 1)
  CU_ASSERT (infinity_test > 20)

  int temp = infinity_test;
  sleep (1);

  CU_ASSERT (temp <= (infinity_test + 2))

  iot_schedule_delete (scheduler, sched3);
  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_delete (void)
{
  iot_threadpool_t *pool = iot_threadpool_alloc (4, 0, NULL, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (pool, logger);

  CU_ASSERT (iot_threadpool_start (pool))
  sum_test = 0;

  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_work3, NULL, IOT_MS_TO_NS (1), 0, 1, NULL);
  iot_schedule_t *sched2 = iot_schedule_create (scheduler, do_work4, NULL, IOT_MS_TO_NS (1), 0, 1, NULL);
  iot_schedule_t *sched3 = iot_schedule_create (scheduler, do_work5, NULL, IOT_MS_TO_NS (1), 0, 0, NULL);
  iot_schedule_t *sched4 = iot_schedule_create (scheduler, do_work3, NULL, IOT_MS_TO_NS (1), 0, 0, NULL);
  iot_schedule_t *sched5 = iot_schedule_create (scheduler, do_work3, NULL, IOT_MS_TO_NS (1), 0, 0, NULL);
  iot_schedule_t *sched6 = iot_schedule_create (scheduler, do_work3, NULL, IOT_MS_TO_NS (1), 0, 0, NULL);

  CU_ASSERT (sched1 != NULL)
  CU_ASSERT (sched2 != NULL)
  CU_ASSERT (sched3 != NULL)
  CU_ASSERT (sched4 != NULL)
  CU_ASSERT (sched5 != NULL)
  CU_ASSERT (sched6 != NULL)
  CU_ASSERT (iot_schedule_add (scheduler, sched1))
  CU_ASSERT (iot_schedule_add (scheduler, sched2))
  CU_ASSERT (iot_schedule_add (scheduler, sched3))
  CU_ASSERT (iot_schedule_add (scheduler, sched4))
  CU_ASSERT (iot_schedule_add (scheduler, sched5))
  CU_ASSERT (iot_schedule_add (scheduler, sched6))

  iot_scheduler_start (scheduler);
  sleep (1);

  iot_schedule_delete (scheduler, sched3);
  iot_scheduler_stop (scheduler);

  CU_ASSERT (sum_test == 1)
  CU_ASSERT (infinity_test > 20)

  int temp = infinity_test;
  iot_scheduler_start (scheduler);
  sleep (1);

  CU_ASSERT (temp == infinity_test)
  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_refcount (void)
{
  iot_threadpool_t *pool = iot_threadpool_alloc (4, 0, NULL, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (pool, logger);
  iot_scheduler_add_ref (scheduler);
  iot_scheduler_free (scheduler);
  iot_scheduler_free (scheduler);
  iot_scheduler_free (NULL);
  iot_threadpool_free (pool);
}

static void cunit_scheduler_iot_scheduler_thread_pool (void)
{
  iot_threadpool_t *pool = iot_threadpool_alloc (4, 0, NULL, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (pool, logger);
  assert (pool == iot_scheduler_thread_pool (scheduler));
  iot_scheduler_free (scheduler);
  iot_threadpool_free (pool);
}

static void cunit_scheduler_nrepeat (void)
{
  iot_threadpool_t *pool = iot_threadpool_alloc (2, 0, NULL, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (pool, logger);
  CU_ASSERT (scheduler != NULL)

  counter = 0;
  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_count, NULL, IOT_MS_TO_NS (100), 0, 5, NULL);
  CU_ASSERT (iot_schedule_add (scheduler, sched1))

  CU_ASSERT (iot_threadpool_start (pool))
  CU_ASSERT (iot_scheduler_start (scheduler))

  sleep (2);

  iot_scheduler_stop (scheduler);
  CU_ASSERT (counter == 5)

  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_starttime (void)
{
  iot_threadpool_t *pool = iot_threadpool_alloc (2, 0, NULL, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (pool, logger);
  CU_ASSERT (scheduler != NULL)

  sum_test = 0;
  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_work4, NULL, IOT_MS_TO_NS (100), IOT_MS_TO_NS (100), 1, NULL);
  CU_ASSERT (iot_schedule_add (scheduler, sched1))
  CU_ASSERT (iot_threadpool_start (pool))
  CU_ASSERT (iot_scheduler_start (scheduler))
  sleep (2);

  iot_scheduler_stop (scheduler);
  CU_ASSERT (sum_test == 1)

  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}

static void cunit_scheduler_setpriority (void)
{
  int prio_max = sched_get_priority_max (SCHED_FIFO);
  int prio_min = sched_get_priority_min (SCHED_FIFO);
  iot_threadpool_t *pool = iot_threadpool_alloc (2, 0, NULL, logger);
  iot_scheduler_t *scheduler = iot_scheduler_alloc (pool, logger);
  CU_ASSERT (scheduler != NULL)

  counter = 0;
  sum_test = 0;
  iot_schedule_t *sched1 = iot_schedule_create (scheduler, do_count, NULL, IOT_MS_TO_NS (10), 0, 100, &prio_max);
  CU_ASSERT (iot_schedule_add (scheduler, sched1))

  iot_schedule_t *sched2 = iot_schedule_create (scheduler, do_work4, NULL, IOT_MS_TO_NS (1000), IOT_MS_TO_NS (1000), 5, &prio_min);
  CU_ASSERT (iot_schedule_add (scheduler, sched2))

  CU_ASSERT (iot_threadpool_start (pool))
  CU_ASSERT (iot_scheduler_start (scheduler))

  sleep (2);

  iot_scheduler_stop (scheduler);

  CU_ASSERT (counter == 100)
  CU_ASSERT (sum_test >= 1)

  iot_threadpool_free (pool);
  iot_scheduler_free (scheduler);
}


extern void cunit_scheduler_test_init ()
{
  CU_pSuite suite = CU_add_suite ("scheduler", suite_init, suite_clean);
  CU_add_test (suite, "scheduler_start", cunit_scheduler_start);
  CU_add_test (suite, "scheduler_stop", cunit_scheduler_stop);
  CU_add_test (suite, "scheduler_create", cunit_scheduler_create);
  CU_add_test (suite, "scheduler_remove", cunit_scheduler_remove);
  CU_add_test (suite, "scheduler_delete", cunit_scheduler_delete);
  CU_add_test (suite, "scheduler_refcount", cunit_scheduler_refcount);
  CU_add_test (suite, "scheduler_nrepeat", cunit_scheduler_nrepeat);
  CU_add_test (suite, "scheduler_starttime", cunit_scheduler_starttime);
  CU_add_test (suite, "scheduler_setpriority", cunit_scheduler_setpriority);
  CU_add_test (suite, "scheduler_iot_scheduler_thread_pool", cunit_scheduler_iot_scheduler_thread_pool);
}
