#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <framework/mlt.h>

mlt_producer create_producer( char *file )
{
	mlt_producer result = NULL;

	// 1st Line preferences
	if ( strstr( file, ".mpg" ) )
		result = mlt_factory_producer( "mcmpeg", file );
	else if ( strstr( file, ".mpeg" ) )
		result = mlt_factory_producer( "mcmpeg", file );
	else if ( strstr( file, ".dv" ) )
		result = mlt_factory_producer( "mcdv", file );
	else if ( strstr( file, ".dif" ) )
		result = mlt_factory_producer( "mcdv", file );
	else if ( strstr( file, ".jpg" ) )
		result = mlt_factory_producer( "pixbuf", file );
	else if ( strstr( file, ".png" ) )
		result = mlt_factory_producer( "pixbuf", file );

	// 2nd Line fallbacks
	if ( result == NULL && strstr( file, ".dv" ) )
		result = mlt_factory_producer( "libdv", file );
	else if ( result == NULL && strstr( file, ".dif" ) )
		result = mlt_factory_producer( "libdv", file );

	return result;
}

mlt_consumer create_consumer( char *id )
{
	char *arg = strchr( id, ':' );
	if ( arg != NULL )
		*arg ++ = '\0';
	return mlt_factory_consumer( id, arg );
}

void track_service( mlt_field field, void *service, mlt_destructor destructor )
{
	mlt_properties properties = mlt_field_properties( field );
	int registered = mlt_properties_get_int( properties, "registered" );
	char *key = mlt_properties_get( properties, "registered" );
	mlt_properties_set_data( properties, key, service, 0, destructor, NULL );
	mlt_properties_set_int( properties, "registered", ++ registered );
}

mlt_filter create_filter( mlt_field field, char *id, int track )
{
	char *arg = strchr( id, ':' );
	if ( arg != NULL )
		*arg ++ = '\0';
	mlt_filter filter = mlt_factory_filter( id, arg );
	if ( filter != NULL )
	{
		mlt_field_plant_filter( field, filter, track );
		track_service( field, filter, ( mlt_destructor )mlt_filter_close );
	}
	return filter;
}

void set_properties( mlt_service service, char *namevalue )
{
	mlt_properties properties = mlt_service_properties( service );
	mlt_properties_parse( properties, namevalue );
}

void transport( mlt_producer producer )
{
	char temp[ 132 ];
	fprintf( stderr, "Press return to continue\n" );
	fgets( temp, 132, stdin );
}

int main( int argc, char **argv )
{
	int i;
	mlt_service  service = NULL;
	mlt_consumer consumer = NULL;
	mlt_multitrack multitrack = NULL;
	mlt_tractor tractor = NULL;
	mlt_producer producer = NULL;
	mlt_playlist playlist = NULL;
	mlt_field field = NULL;

	// Construct the factory
	mlt_factory_init( getenv( "MLT_REPOSITORY" ) );

	// Set up containers
	playlist = mlt_playlist_init( );
	multitrack = mlt_multitrack_init( );
	tractor = mlt_tractor_init( );

	// Field must be connected on construction
	field = mlt_field_init( mlt_multitrack_service( multitrack ) );
	mlt_properties properties = mlt_field_properties( field );
	mlt_properties_set_int( properties, "registered", 0 );

	// Parse the arguments
	for ( i = 1; i < argc; i ++ )
	{
		if ( !strcmp( argv[ i ], "-consumer" ) )
		{
			consumer = create_consumer( argv[ ++ i ] );
			if ( consumer != NULL )
				service = mlt_consumer_service( consumer );
		}
		else if ( !strcmp( argv[ i ], "-filter" ) )
		{
			mlt_filter filter = create_filter( field, argv[ ++ i ], 0 );
			if ( filter != NULL )
				service = mlt_filter_service( filter );
		}
		else if ( !strstr( argv[ i ], "=" ) )
		{
			if ( producer != NULL )
				mlt_playlist_append( playlist, producer );
			producer = create_producer( argv[ i ] );
			if ( producer != NULL )
				service = mlt_producer_service( producer );
		}
		else
		{
			set_properties( service, argv[ i ] );
		}
	}

	// If we have no consumer, default to sdl
	if ( consumer == NULL )
		consumer= mlt_factory_consumer( "sdl", NULL );

	// Connect producer to playlist
	mlt_playlist_append( playlist, producer );

	// Connect multitrack to producer
	mlt_multitrack_connect( multitrack, mlt_playlist_producer( playlist ), 0 );

	// Connect tractor to field
	mlt_tractor_connect( tractor, mlt_field_service( field ) );

	// Connect consumer to tractor
	mlt_consumer_connect( consumer, mlt_tractor_service( tractor ) );

	// Transport functionality
	transport( producer );

	// Close the services
	mlt_consumer_close( consumer );
	mlt_tractor_close( tractor );
	mlt_field_close( field );
	mlt_multitrack_close( multitrack );
	mlt_producer_close( producer );

	// Close the factory
	mlt_factory_close( );

	return 0;
}
