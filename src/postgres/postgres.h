#pragma once
#include <pqxx/connection>
#include <pqxx/transaction>
#include <string>
#include <vector>

#include "../domain/author.h"
#include "../domain/book.h"
#include "../app/unit_of_work.h"

namespace postgres {


class AuthorRepositoryImpl : public domain::AuthorRepository {
public:
    explicit AuthorRepositoryImpl(pqxx::connection& connection, pqxx::work& work)
        : connection_{connection}, work_{work}{
    }

    void Save(const domain::Author& author) override;
    std::vector<std::pair<std::string, std::string>> Read() override;
    void DeleteByName(const std::string& author_name) override;
    void DeleteById(const std::string& author_id) override;
    void EditByName(const std::string& old_name, const std::string& new_name) override;
    void EditById(const std::string& id, const std::string& new_name) override;

private:
    pqxx::connection& connection_;
    pqxx::work& work_;
};

class BookRepositoryImpl : public domain::BookRepository {
public:
    explicit BookRepositoryImpl(pqxx::connection& connection, pqxx::work& work)
            : connection_{connection}, work_{work}{
    }

    void Save(const domain::Book& book) override;
    std::vector<domain::BookData> Read() override;
    std::vector<domain::BookData> ReadByName(const std::string& book_name) override;
    domain::BookData ReadById(const std::string& book_id) override;
    std::vector<domain::BookData> ReadAuthorBooks(const std::string& author_id) override;

    void DeleteByName(const std::string& book_name ) override;
    void DeleteById(const std::string& book_id ) override;

    void EditTitleById(const std::string& id, const std::string& new_name) override;
    void EditYearById(const std::string& id, int new_year) override;

private:
    pqxx::connection& connection_;
    pqxx::work& work_;
};

class BookTagsRepositoryImpl : public domain::BookTagsRepository {
public:
    explicit BookTagsRepositoryImpl(pqxx::connection& connection, pqxx::work& work)
            : connection_{connection}, work_{work}{
    }

    void Save(const domain::BookTags& book_tags) override;
    std::vector<std::pair<std::string, std::string>> Read() override;
    std::vector<std::string> ReadById(const std::string& book_id) override;
    void Update(const domain::BookTags& book_tags) override;
    void DeleteById(const std::string& book_id) override;

private:
    pqxx::connection& connection_;
    pqxx::work& work_;
};

//======================================UnitOfWorkImpl============================
//--------------------------------------------------------------------------------
    class UnitOfWorkImpl : public app::UnitOfWork {
    public:
        explicit UnitOfWorkImpl(pqxx::connection& connection)
                : connection_(connection), work_(connection_),
                  authors_(new AuthorRepositoryImpl(connection_, work_)),
                  books_(new BookRepositoryImpl(connection_, work_)),
                  book_tags_(new BookTagsRepositoryImpl(connection_, work_)) {
        }


        domain::AuthorRepository* Author() override{
            return authors_;
        }
        domain::BookRepository* Book() override{
            return books_;
        }
        domain::BookTagsRepository* BookTags() override{
            return book_tags_;
        }
        void Commit() override{
            work_.commit();
        }

    private:
        pqxx::connection& connection_;
        pqxx::work work_;
        domain::AuthorRepository* authors_;
        domain::BookRepository* books_;
        domain::BookTagsRepository* book_tags_;
    };

    class UnitOfWorkFactoryImpl : public app::UnitOfWorkFactory {
    public:
        explicit UnitOfWorkFactoryImpl(pqxx::connection& connection)
                : connection_(connection){}

        app::UnitOfWork* CreateUnitOfWork() override{
            return new UnitOfWorkImpl(connection_);
        }

    private:
        pqxx::connection& connection_;
    };


class Database {
public:
    explicit Database(pqxx::connection connection);

    /*AuthorRepositoryImpl& GetAuthors() & {
        return authors_;
    }

    BookRepositoryImpl& GetBooks() & {
        return books_;
    }

    BookTagsRepositoryImpl& GetBookTags() & {
        return book_tags_;
    }*/

    pqxx::connection& GetConnection(){
        return connection_;
    }

private:
    pqxx::connection connection_;
    /*AuthorRepositoryImpl authors_{connection_};
    BookRepositoryImpl books_{connection_};
    BookTagsRepositoryImpl book_tags_{connection_};*/
};

}  // namespace postgres