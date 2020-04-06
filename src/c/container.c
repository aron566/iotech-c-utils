//
// Copyright (c) 2019 IOTech
//
// SPDX-License-Identifier: Apache-2.0
//
#include "iot/container.h"
#include "iot/logger.h"
#ifdef IOT_BUILD_DYNAMIC_LOAD
#include <dlfcn.h>
#endif

#define IOT_COMPONENT_DELTA 4

typedef struct iot_dlhandle_holder_t
{
  struct iot_dlhandle_holder_t * next;
  void * load_handle;
} iot_dlhandle_holder_t;

typedef struct iot_component_holder_t
{
  iot_component_t * component;
  const iot_component_factory_t * factory;
  char * name;
} iot_component_holder_t;

struct iot_container_t
{
  iot_logger_t * logger;
  iot_dlhandle_holder_t * handles;
  iot_component_holder_t ** components;
  uint32_t ccount;
  uint32_t csize;
  char * name;
  pthread_rwlock_t lock;
  iot_container_t * next;
  iot_container_t * prev;
};

static const iot_component_factory_t * iot_component_factories = NULL;
static iot_container_t * iot_containers = NULL;
#ifdef __ZEPHYR__
  static PTHREAD_MUTEX_DEFINE (iot_container_mutex);
#else
  static pthread_mutex_t iot_container_mutex = PTHREAD_MUTEX_INITIALIZER;
#endif

static iot_container_t * iot_container_find_locked (const char * name)
{
  assert (name);
  iot_container_t * cont = iot_containers;
  while (cont)
  {
    if (strcmp (cont->name, name) == 0) break;
    cont = cont->next;
  }
  return cont;
}
iot_container_t * iot_container_alloc (const char * name)
{
  iot_container_t * cont = NULL;
  pthread_mutex_lock (&iot_container_mutex);
  if (iot_container_find_locked (name) == NULL)
  {
    cont = calloc (1, sizeof (*cont));
    cont->components = calloc (IOT_COMPONENT_DELTA, sizeof (iot_component_holder_t));
    cont->csize = IOT_COMPONENT_DELTA;
    cont->name = strdup (name);
    pthread_rwlock_init (&cont->lock, NULL);
    cont->next = iot_containers;
    if (iot_containers) iot_containers->prev = cont;
    iot_containers = cont;
  }
  pthread_mutex_unlock (&iot_container_mutex);
  return cont;
}

#ifdef IOT_BUILD_DYNAMIC_LOAD
static void iot_container_add_handle (iot_container_t * cont,  void * handle)
{
  assert (cont && handle);
  iot_dlhandle_holder_t * holder = malloc (sizeof (*holder));
  holder->load_handle = handle;
  pthread_rwlock_wrlock (&cont->lock);
  holder->next = cont->handles;
  cont->handles = holder;
  pthread_rwlock_unlock (&cont->lock);
}
#endif

bool iot_container_init (iot_container_t * cont, iot_container_config_t * conf)
{
  assert (conf);
  bool ret = true;
  char * config = (conf->load) (cont->name, conf->from);
  assert (config);
  iot_data_t * map = iot_data_from_json (config);
  iot_data_map_iter_t iter;
  iot_data_map_iter (map, &iter);
  free (config);

#ifdef IOT_BUILD_DYNAMIC_LOAD

  // pre-pass to find the factory to be added to support dynamic loading of libraries
  while (iot_data_map_iter_next (&iter))
  {
    const char *cname = iot_data_map_iter_string_key(&iter);
    const iot_component_factory_t *factory = NULL;
    void *handle = NULL;

    const char * ctype = iot_data_map_iter_string_value (&iter);
    factory = iot_component_factory_find (ctype);
    if (!factory)
    {
      config = (conf->load) (cname, conf->from);
      if (config)
      {
        iot_data_t * cmap = iot_data_from_json (config);
        const iot_data_t * value = iot_data_string_map_get (cmap, "Library");
        if (value)
        {
          const char *library_name = (const char *) (iot_data_string (value));
          handle = dlopen (library_name, RTLD_LAZY);
          if (handle)
          {
            // find the symbol and call config_fn
            value = iot_data_string_map_get (cmap, "Factory");
            if (value)
            {
              const char * factory_name = (const char *) (iot_data_string (value));
              const iot_component_factory_t *(*factory_fn) (void);
              factory_fn = dlsym (handle, factory_name);
              if (factory_fn)
              {
                factory = factory_fn ();
                iot_component_factory_add (factory);
                iot_container_add_handle (cont, handle);
              }
              else
              {
                fprintf (stderr, "ERROR: Invalid configuration, Incorrect Factory name\n");
              }
            }
            else
            {
              fprintf (stderr, "ERROR: Incomplete configuration, Factory name not available\n");
              dlclose (handle);
            }
          }
          else
          {
            fprintf (stderr, "ERROR: Incomplete configuration, Library name not available\n");
          }
        }
        iot_data_free (cmap);
      }
      free (config);
    }
  }
#endif

  while (iot_data_map_iter_next (&iter))
  {
    const char * cname = iot_data_map_iter_string_key (&iter);
    const char * ctype = iot_data_map_iter_string_value (&iter);
    const iot_component_factory_t * factory = iot_component_factory_find (ctype);
    if (factory)
    {
      config = (conf->load) (cname, conf->from);
      if (config)
      {
        iot_data_t * cmap = iot_data_from_json (config);
        iot_component_t * comp = (factory->config_fn) (cont, cmap);
        iot_data_free (cmap);
        free (config);
        if (comp)
        {
          iot_component_holder_t * ch = malloc (sizeof (*ch));
          ch->component = comp;
          ch->name = strdup (cname);
          ch->factory = factory;
          if ((cont->ccount + 1) == cont->csize)
          {
            cont->csize += IOT_COMPONENT_DELTA;
            cont->components = realloc (cont->components, cont->csize * sizeof (iot_component_holder_t));
          }
          cont->components[cont->ccount++] = ch;
        }
      }
    }
  }
  iot_data_free (map);
  return ret;
}

void iot_container_free (iot_container_t * cont)
{
  if (cont)
  {
    pthread_mutex_lock (&iot_container_mutex);
    if (cont->next) cont->next->prev = cont->prev;
    if (cont->prev)
    {
      cont->prev->next = cont->next;
    }
    else
    {
      iot_containers = cont->next;
    }
    pthread_mutex_unlock (&iot_container_mutex);
    while (cont->ccount)
    {
      iot_component_holder_t *ch = cont->components[--cont->ccount]; // Free in reverse of declaration order (dependents last)
      (ch->factory->free_fn) (ch->component);
      free (ch->name);
      free (ch);
    }
    free (cont->components);
#ifdef IOT_BUILD_DYNAMIC_LOAD
    while (cont->handles)
    {
      iot_dlhandle_holder_t *dl_handles = cont->handles;
      cont->handles = dl_handles->next;
      dlclose (dl_handles->load_handle);
      free (dl_handles);
    }
#endif
    pthread_rwlock_destroy (&cont->lock);
    free (cont->name);
    free (cont);
  }
}

bool iot_container_start (iot_container_t * cont)
{
  bool ret = true;
  pthread_rwlock_rdlock (&cont->lock);
  for (uint32_t i = 0; i < cont->ccount; i++) // Start in declaration order (dependents first)
  {
    iot_component_t * comp = cont->components[i]->component;
    ret = ret && (comp->start_fn) (comp);
  }
  pthread_rwlock_unlock (&cont->lock);
  return ret;
}

void iot_container_stop (iot_container_t * cont)
{
  pthread_rwlock_rdlock (&cont->lock);
  for (int32_t i = cont->ccount - 1; i >= 0; i--) // Stop in reverse of declaration order (dependents last)
  {
    iot_component_t * comp = cont->components[i]->component;
    (comp->stop_fn) (comp);
  }
  pthread_rwlock_unlock (&cont->lock);
}

static const iot_component_factory_t * iot_component_factory_find_locked (const char * type)
{
  assert (type);
  const iot_component_factory_t * iter = iot_component_factories;
  while (iter)
  {
    if (strcmp (iter->type, type) == 0) break;
    iter = iter->next;
  }
  return iter;
}

void iot_component_factory_add (const iot_component_factory_t * factory)
{
  assert (factory);
  pthread_mutex_lock (&iot_container_mutex);
  if (iot_component_factory_find_locked (factory->type) == NULL)
  {
    ((iot_component_factory_t*) factory)->next = iot_component_factories;
    iot_component_factories = factory;
  }
  pthread_mutex_unlock (&iot_container_mutex);
}

extern const iot_component_factory_t * iot_component_factory_find (const char * type)
{
  pthread_mutex_lock (&iot_container_mutex);
  const iot_component_factory_t * factory = iot_component_factory_find_locked (type);
  pthread_mutex_unlock (&iot_container_mutex);
  return factory;
}

extern iot_container_t * iot_container_find (const char * name)
{
  pthread_mutex_lock (&iot_container_mutex);
  iot_container_t * cont = iot_container_find_locked (name);
  pthread_mutex_unlock (&iot_container_mutex);
  return cont;
}

iot_component_t * iot_container_find_component (iot_container_t * cont, const char * name)
{
  assert (cont);
  iot_component_t * comp = NULL;
  if (name)
  {
    pthread_rwlock_rdlock (&cont->lock);
    for (uint32_t i = 0; i < cont->ccount; i++)
    {
      if (strcmp (cont->components[i]->name, name) == 0)
      {
        comp = cont->components[i]->component;
        break;
      }
    }
    pthread_rwlock_unlock (&cont->lock);
  }
  return comp;
}
