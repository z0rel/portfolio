#include <string>

#include <http_handler.h>
#include <uni_log_impl.h>
#include <base_interfaces/uni_logger_module.h>
#include <roc_parser.h>

using namespace itcs;
using namespace roc;


int main( int argc, char* argv[] )
{
     try
     {
          std::string configFileName;
          if ( argc == 2 )
               configFileName = argv[ 1 ];


          initLogger( ITCS_LOGLEVEL_TRACE );
          TRACE( "main begin" );
          {
               std::shared_ptr< HttpHandler > handler = std::make_shared< HttpHandler >( configFileName );
               handler->mainLoop();
          }
          TRACE( "main end" );
     }
     catch ( std::exception& err )
     {
          ERROR( "ERROR: main() - exception: " << err.what() << "\n" );
     }
     return 0;
}
