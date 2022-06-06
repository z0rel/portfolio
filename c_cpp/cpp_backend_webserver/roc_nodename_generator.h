/// @file
/// @brief Генератор имени узла по его шаблону

#pragma once
#ifndef NODENAMEGENERATOR_H
#define NODENAMEGENERATOR_H

#include <memory>
#include <string>
#include <vector>

#include <http_handler_settings.h>

namespace itcs
{
namespace roc
{


struct NodenameGrammar;

/// Элемент опциональной подстановки для генерации узла
struct NodeTemplateItem {
    /// Категория элемента имени узла
    enum Cathegory
    {
         /// Логин
         LOGIN,
         /// Описание пользователя
         DESCRIPTION,
         /// Платформа
         PLATFORM,
         /// Текущее время в секундах (поле для упрощения тестирования)
         TIMESTAMP,
         /// Обычный текст
         TEXT
    };

    /// Элемент имени - либо текст, либо подстановка из поля Cathegory
    typedef std::pair< char, Cathegory > ItemPart;

    explicit NodeTemplateItem();

    NodeTemplateItem( bool opt, NodeTemplateItem& oth );

    NodeTemplateItem( std::vector< ItemPart >& parts );


    /// Является ли элемент шаблона необязательным
    bool optional  = false;

    /// Первая категория в квадратных скобках. По ней будет определяться - выполнять подстановку или нет
    Cathegory firstCathegory = TEXT;

    /// Подэлементы имени
    std::vector< ItemPart > itemParts = std::vector< ItemPart >();
};


/// Генератор имен узлов
class NodenameGenerator
{
public:
     typedef std::vector< NodeTemplateItem > TemplateSyntaxStruct;

     explicit NodenameGenerator( std::shared_ptr< HttpHandlerSettings >& settings );
     ~NodenameGenerator();

     /// Сгенерировать имя узла по заданным логину, описанию и паролю. Исходные данные должны быть обрезаны и дополнены.
     void generate( std::string& dest, const std::string& login, const std::string& descr, const std::string& os );

private:
     /// Синтаксическая структура шаблона подстановок для генерации по ней имен узлов
     TemplateSyntaxStruct templateSyntaxStruct_ = TemplateSyntaxStruct();

     /// Максимальная длина имени узла
     unsigned int maximalNodenameLength_ = 0;
};


} // namespace roc
} // namespace itcs


#endif // NODENAMEGENERATOR_H
