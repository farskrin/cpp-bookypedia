#pragma once

#include "../domain/author_fwd.h"

namespace app {

    class UnitOfWork {
    public:
        virtual domain::AuthorRepository* Author() = 0;
        virtual domain::BookRepository* Book() = 0;
        virtual domain::BookTagsRepository* BookTags() = 0;
        virtual void Commit() = 0;

    protected:
        ~UnitOfWork() = default;
    };

    class UnitOfWorkFactory{
    public:
        virtual UnitOfWork* CreateUnitOfWork() = 0;

    protected:
        ~UnitOfWorkFactory() = default;
    };

}