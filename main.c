#include "vomit.h"

int
main( int argc, char **argv )
{
	vomit_init( argc, argv );

	for( ;; )
		cpu_main();
}
