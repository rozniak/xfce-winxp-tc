#ifndef __PLACESLIST_H__
#define __PLACESLIST_H__

#include <glib.h>
#include <gtk/gtk.h>

G_BEGIN_DECLS

//
// GTK OOP BOILERPLATE
//
typedef struct _PlacesListClass PlacesListClass;
typedef struct _PlacesList      PlacesList;

#define TYPE_PLACES_LIST            (places_list_get_type())
#define PLACES_LIST(obj)            (G_TYPE_CHECK_INSTANCE_CAST((obj), TYPE_PLACES_LIST, PlacesList))
#define PLACES_LIST_CLASS(klass)    (G_TYPE_CHECK_CLASS_CAST((klass), TYPE_PLACES_LIST, PlacesListClass))
#define IS_PLACES_LIST(obj)         (G_TYPE_CHECK_INSTANCE_TYPE((obj), TYPE_PLACES_LIST))
#define IS_PLACES_LIST_CLASS(klass) (G_TYPE_CHECK_CLASS_TYPE((klass), TYPE_PLACES_LIST))
#define PLACES_LIST_GET_CLASS       (G_TYPE_INSTANCE_GET_CLASS((obj), TYPE_PLACES_LIST, PlacesListClass))

GType places_list_get_type(void) G_GNUC_CONST;

G_END_DECLS

#endif
