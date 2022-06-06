#include "roc_nodename_generator.h"

#pragma GCC diagnostic push

#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wextra"
#pragma GCC diagnostic ignored "-Weffc++"
#pragma GCC diagnostic ignored "-Wconversion"
#pragma GCC diagnostic ignored "-Wold-style-cast"
#pragma GCC diagnostic ignored "-Wctor-dtor-privacy"
#pragma GCC diagnostic ignored "-Wunused-local-typedefs"

#include <boost/fusion/include/io.hpp>
#include <boost/fusion/include/std_pair.hpp>
#include <boost/lambda/lambda.hpp>
#include <boost/spirit/include/phoenix_core.hpp>
#include <boost/spirit/include/phoenix_object.hpp>
#include <boost/spirit/include/phoenix_operator.hpp>
#include <boost/spirit/include/phoenix_bind.hpp>
#include <boost/spirit/include/qi.hpp>
#include <boost/spirit/include/qi_eoi.hpp>
#include <boost/type_traits.hpp>

#pragma GCC diagnostic pop

#include <chrono>

#include <base_interfaces/uni_logger.h>


namespace itcs
{
namespace roc
{


namespace qi     = boost::spirit::qi;
namespace spirit = boost::spirit;
namespace phx    = boost::phoenix;


struct NodenameGrammar
{
     typedef std::string::const_iterator Iterator;
     typedef qi::rule< Iterator, std::vector< NodeTemplateItem >() > StartRule;
     typedef qi::rule< Iterator, NodeTemplateItem() >                TemplateItems;
     typedef qi::rule< Iterator, NodeTemplateItem::ItemPart() >      TemplateItem;
     typedef qi::symbols< char, NodeTemplateItem::Cathegory >        SymCathegory;
     typedef qi::rule< Iterator, char() >                            RuleChar;


     explicit NodenameGrammar();


     StartRule     start                 = StartRule();
     TemplateItems templateItem          = TemplateItems();
     TemplateItem  templateInternalItem  = TemplateItem();
     TemplateItems templateInternalItems = TemplateItems();
     SymCathegory  symCathegory          = SymCathegory();
     RuleChar      singleCh              = RuleChar();
};


NodenameGrammar::NodenameGrammar()
{
     using boost::spirit::qi::lit;
     using boost::spirit::qi::char_;
     using boost::spirit::_val;
     using boost::spirit::qi::repeat;

     try
     {
          start = ( + templateItem );

          templateItem =
                    ( "[" >> templateInternalItems >> "]" )
                    [
                         _val = phx::construct< NodeTemplateItem >( true, qi::_1 )
                    ]
              |     templateInternalItems [ _val = qi::_1 ]
              ;
          templateInternalItems = ( + templateInternalItem ) [ _val = phx::construct< NodeTemplateItem >( qi::_1 ) ];

          singleCh = ~char_( "[]" );
          templateInternalItem =
                    symCathegory
                    [
                         _val = phx::construct< NodeTemplateItem::ItemPart >( '\0', qi::_1 )
                    ]
              |     singleCh
                    [
                         _val = phx::construct< NodeTemplateItem::ItemPart >( qi::_1, NodeTemplateItem::TEXT )
                    ]
                    ;

          symCathegory.add
               ( "login"    , NodeTemplateItem::LOGIN       )
               ( "desc"     , NodeTemplateItem::DESCRIPTION )
               ( "platform" , NodeTemplateItem::PLATFORM    )
               ( "timestamp", NodeTemplateItem::TIMESTAMP   );

     }
     catch ( std::exception& err )
     {
          ITCS_ERROR( "ERROR: NodenameGrammar::NodenameGrammar() - exception: " << err.what() << "\n" );
     }
}


NodeTemplateItem::NodeTemplateItem()
{

}


NodeTemplateItem::NodeTemplateItem( bool opt, NodeTemplateItem& oth )
{
     optional = opt;
     firstCathegory = oth.firstCathegory;
     itemParts.swap( oth.itemParts );
}


NodeTemplateItem::NodeTemplateItem( std::vector< ItemPart >& parts )
{
     itemParts.swap( parts );
}


NodenameGenerator::NodenameGenerator( std::shared_ptr< HttpHandlerSettings >& settings )
{
     try
     {
          std::string nodeTemplate = settings->nodenameTemplate();
          maximalNodenameLength_ = settings->nodenameMaxlen();
          /// Грамматика Spirit::Qi для разбора шаблона генератора узлов
          NodenameGrammar g;
          NodenameGrammar::Iterator iter = nodeTemplate.begin();
          NodenameGrammar::Iterator end  = nodeTemplate.end();
          qi::parse( iter, end, g.start, templateSyntaxStruct_ );
          if ( iter != end )
          {
               ITCS_ERROR( "ERROR: NodenameGenerator::NodenameGenerator() - template parsing error: " );
          }

          for ( NodeTemplateItem& val : templateSyntaxStruct_ )
          {
               for ( NodeTemplateItem::ItemPart& part : val.itemParts )
               {
                    if ( part.second != NodeTemplateItem::TEXT )
                    {
                         val.firstCathegory = part.second;
                         break;
                    }
               }
          }
     }
     catch ( std::exception& err )
     {
          ITCS_ERROR( "ERROR: NodenameGenerator::NodenameGenerator() - exception: " << err.what() << "\n" );
     }
}


NodenameGenerator::~NodenameGenerator()
{

}


void NodenameGenerator::generate( std::string& dest,
                                  const std::string& login,
                                  const std::string& descr,
                                  const std::string& os )
{
     for ( NodeTemplateItem& val : templateSyntaxStruct_ )
     {
          if ( val.optional )
          {
               switch ( val.firstCathegory )
               {
                    case NodeTemplateItem::DESCRIPTION:
                         if ( descr.empty() )
                         {
                              continue;
                         }

                         break;

                    case NodeTemplateItem::LOGIN:
                         if ( login.empty() )
                         {
                              continue;
                         }
                         break;

                    case NodeTemplateItem::PLATFORM:
                         if ( os.empty() )
                         {
                              continue;
                         }
                         break;

                    default:
                         break;
               }
          }

          for ( NodeTemplateItem::ItemPart& part : val.itemParts )
          {
               switch ( part.second )
               {
                    case NodeTemplateItem::DESCRIPTION:
                         dest.append( descr );
                         break;

                    case NodeTemplateItem::LOGIN:
                         dest.append( login );
                         break;

                    case NodeTemplateItem::PLATFORM:
                         dest.append( os );
                         break;

                    case NodeTemplateItem::TIMESTAMP:
                         dest.append( std::to_string( std::time( nullptr ) ) + std::to_string( clock() ) );
                         break;

                    case NodeTemplateItem::TEXT:
                         dest.push_back( part.first );
                         break;
               }
          }
     }
     if ( dest.length() > maximalNodenameLength_ )
     {
          dest.erase( dest.begin() + maximalNodenameLength_, dest.end() );
     }
}


} // namespace roc
} // namespace itcs
