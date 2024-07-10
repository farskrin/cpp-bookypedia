#include "postgres.h"

#include <pqxx/zview.hxx>
#include <pqxx/pqxx>

namespace postgres {
    namespace {
        // for string delimiter
        std::vector<std::string> split(std::string s, std::string delimiter) {
            size_t pos_start = 0, pos_end, delim_len = delimiter.length();
            std::string token;
            std::vector<std::string> res;

            while ((pos_end = s.find(delimiter, pos_start)) != std::string::npos) {
                token = s.substr (pos_start, pos_end - pos_start);
                pos_start = pos_end + delim_len;
                res.push_back (token);
            }

            res.push_back (s.substr (pos_start));
            return res;
        }
    }

    using namespace std::literals;
    using pqxx::operator"" _zv;

    //------------------------------------------------------------------------
    //===============AuthorRepositoryImpl==================================
    void AuthorRepositoryImpl::Save(const domain::Author& author) {
        // Пока каждое обращение к репозиторию выполняется внутри отдельной транзакции
        // В будущих уроках вы узнаете про паттерн Unit of Work, при помощи которого сможете несколько
        // запросов выполнить в рамках одной транзакции.
        // Вы также может самостоятельно почитать информацию про этот паттерн и применить его здесь.
        //pqxx::work work{connection_};
        work_.exec_params(
            R"(INSERT INTO authors (id, name) VALUES ($1, $2) ON CONFLICT (id) DO UPDATE SET name=$2;)"_zv,
            author.GetId().ToString(), author.GetName());
    }

    std::vector<std::pair<std::string, std::string>> AuthorRepositoryImpl::Read() {
        auto query_text = "SELECT id, name FROM authors ORDER BY name;"_zv;
        std::vector<std::pair<std::string, std::string>> authors;
        for (auto [id, name] : work_.query<std::string, std::string>(query_text)) {
            authors.emplace_back(id, name);
        }

        return authors;
    }

    void AuthorRepositoryImpl::DeleteByName(const std::string &author_name) {

        auto query_author = "SELECT id, name FROM authors WHERE name='" + author_name;
        query_author += "' LIMIT 1;"_zv;
        auto [id, name] = work_.query1<std::string, std::string>(query_author);
        std::string author_id = id;
        //-----
        auto query_book = "SELECT id FROM books WHERE author_id='" + author_id;
        query_book += "' LIMIT 1;"_zv;
        std::optional result_book = work_.query01<std::string>(query_book);
        if (result_book) {
            //TODO: loop for book author
            auto query_book_all = "SELECT id FROM books WHERE author_id='" + author_id + "';";
            for(auto [id] : work_.query<std::string>(query_book_all)){
                std::string book_id = id;
                auto query_book_tags = "SELECT book_id FROM book_tags WHERE book_id='" + book_id;
                query_book_tags += "' LIMIT 1;"_zv;
                std::optional result_book_tags = work_.query01<std::string>(query_book_tags);
                if (result_book_tags) {
                    work_.exec_params(R"(DELETE FROM book_tags WHERE book_id=$1;)"_zv, book_id);
                }
                work_.exec_params(R"(DELETE FROM books WHERE id=$1;)"_zv,book_id);
            }
        }
        work_.exec_params(R"(DELETE FROM authors WHERE name=$1;)"_zv,author_name);
    }

    void AuthorRepositoryImpl::DeleteById(const std::string &a_id) {

        auto query_author = "SELECT id, name FROM authors WHERE id='" + a_id;
        query_author += "' LIMIT 1;"_zv;
        auto [author_id, name] = work_.query1<std::string, std::string>(query_author);
        //-----
        auto query_book = "SELECT id FROM books WHERE author_id='" + author_id;
        query_book += "' LIMIT 1;"_zv;
        std::optional result_book = work_.query01<std::string>(query_book);
        if (result_book) {
            auto query_book_all = "SELECT id FROM books WHERE author_id='" + author_id + "';";
            for(auto [id] : work_.query<std::string>(query_book_all)){
                std::string book_id = id;
                auto query_book_tags = "SELECT book_id FROM book_tags WHERE book_id='" + book_id;
                query_book_tags += "' LIMIT 1;"_zv;
                std::optional result_book_tags = work_.query01<std::string>(query_book_tags);
                if (result_book_tags) {
                    work_.exec_params(R"(DELETE FROM book_tags WHERE book_id=$1;)"_zv, book_id);
                }
                work_.exec_params(R"(DELETE FROM books WHERE id=$1;)"_zv,book_id);
            }
        }
        work_.exec_params(R"(DELETE FROM authors WHERE id=$1;)"_zv,a_id);
    }

    void AuthorRepositoryImpl::EditByName(const std::string &old_name, const std::string &new_name) {
        auto query_author = "SELECT id, name FROM authors WHERE name='" + old_name;
        query_author += "' LIMIT 1;"_zv;
        auto [id, name] = work_.query1<std::string, std::string>(query_author);
        //---
        work_.exec_params(R"(UPDATE authors SET name=$2 WHERE name=$1;)"_zv, old_name, new_name);
    }

    void AuthorRepositoryImpl::EditById(const std::string &a_id, const std::string &new_name) {
        auto query_author = "SELECT id, name FROM authors WHERE id='" + a_id;
        query_author += "' LIMIT 1;"_zv;
        auto [id, name] = work_.query1<std::string, std::string>(query_author);
        //---
        work_.exec_params(R"(UPDATE authors SET name=$2 WHERE id=$1;)"_zv, a_id, new_name);
    }


    //------------------------------------------------------------------------
    //===============BookRepositoryImpl==================================
    void BookRepositoryImpl::Save(const domain::Book &book) {
        work_.exec_params(
                R"(INSERT INTO books (id, author_id, title, publication_year)
                     VALUES ($1, $2, $3, $4);)"_zv,
                book.GetId().ToString(), book.GetAuthorId(), book.GetTitle(), book.GetYear());
    }

    std::vector<domain::BookData> BookRepositoryImpl::Read() {
        std::vector<domain::BookData> books;
        auto query_text = "SELECT books.id AS book_id, author_id, authors.name AS name, title, publication_year FROM books INNER JOIN authors ON authors.id = author_id ORDER BY title, name, publication_year;"_zv;
        for (auto [id, author_id, author_name, title, publication_year]
                : work_.query<std::string, std::string, std::string, std::string, int>(query_text)) {
            books.emplace_back(id, author_id, author_name, title, publication_year);
        }

        return books;
    }

    std::vector<domain::BookData> BookRepositoryImpl::ReadAuthorBooks(const std::string &author_id) {
        std::vector<domain::BookData> books;
        auto query_text = "SELECT id, author_id, title, publication_year FROM books WHERE author_id='" + author_id;
        query_text += "' ORDER BY publication_year, title;"_zv;
        for (auto [id, author_id, title, publication_year] : work_.query<std::string, std::string, std::string, int>(query_text)) {
            std::string author_name;
            books.emplace_back(id, author_id, author_name, title, publication_year);
        }

        return books;
    }

    //Read books by title
    std::vector<domain::BookData> BookRepositoryImpl::ReadByName(const std::string &book_name) {
        std::vector<domain::BookData> books;
        auto query_text = "SELECT books.id AS book_id, author_id, authors.name AS name, title, publication_year"
            " FROM books INNER JOIN authors ON authors.id = author_id WHERE title='" + book_name;
        query_text += "' ORDER BY title, name, publication_year;"_zv;
        for (auto [id, author_id, author_name, title, publication_year]
                        : work_.query<std::string, std::string, std::string, std::string, int>(query_text)) {
            books.emplace_back(id, author_id, author_name, title, publication_year);
        }

        return books;
    }

    void BookRepositoryImpl::DeleteByName(const std::string &book_name) {

        work_.exec_params(R"(DELETE FROM books WHERE title=$1;)"_zv,book_name);

    }

    void BookRepositoryImpl::DeleteById(const std::string &book_id) {

        work_.exec_params(R"(DELETE FROM books WHERE id=$1;)"_zv,book_id);

    }

    domain::BookData BookRepositoryImpl::ReadById(const std::string &book_id) {
        domain::BookData book_data;
        auto query_book = "SELECT books.id AS book_id, author_id, authors.name AS name, title, publication_year"
                          " FROM books INNER JOIN authors ON authors.id = author_id WHERE books.id='" + book_id;
        query_book += "';"_zv;

        std::optional result_book = work_.query01<std::string, std::string, std::string, std::string, int>(query_book);
        if (result_book) {
            auto [id, author_id, author_name, title, publication_year] = *result_book;
            book_data = {id, author_id, author_name, title, publication_year};
        }
        return book_data;
    }

    void BookRepositoryImpl::EditTitleById(const std::string &b_id, const std::string &new_name) {
        auto query_book = "SELECT id, title FROM books WHERE id='" + b_id;
        query_book += "' LIMIT 1;"_zv;
        auto [id, name] = work_.query1<std::string, std::string>(query_book);
        //---
        work_.exec_params(R"(UPDATE books SET title=$2 WHERE id=$1;)"_zv, b_id, new_name);
    }

    void BookRepositoryImpl::EditYearById(const std::string &b_id, int new_year) {
        auto query_book = "SELECT id, title FROM books WHERE id='" + b_id;
        query_book += "' LIMIT 1;"_zv;
        auto [id, name] = work_.query1<std::string, std::string>(query_book);
        //---
        work_.exec_params(R"(UPDATE books SET publication_year=$2 WHERE id=$1;)"_zv, b_id, new_year);
    }

    //------------------------------------------------------------------------
    //===============BookTagsRepositoryImpl==================================
    void BookTagsRepositoryImpl::Save(const domain::BookTags &book_tags) {
        for(const auto& tag : book_tags.GetTags()){
            work_.exec_params(R"(INSERT INTO book_tags (book_id, tag)
                     VALUES ($1, $2);)"_zv, book_tags.GetBookId(), tag);
        }

    }

    std::vector<std::pair<std::string, std::string>> BookTagsRepositoryImpl::Read() {
        std::vector<std::pair<std::string, std::string>> book_tags;
        auto query_text = "SELECT book_id, tag FROM book_tags;"_zv;
        for (auto [book_id, tag] : work_.query<std::string, std::string>(query_text)) {
            book_tags.emplace_back(book_id, tag);
        }
        return book_tags;
    }

    void BookTagsRepositoryImpl::Update(const domain::BookTags &book_tags) {
        //TODO: delete old and insert new book_tags
        auto query_text = "DELETE FROM book_tags WHERE book_id='" + book_tags.GetBookId();
        query_text += "';"_zv;
        work_.exec(query_text);
        //---
        for(const auto& tag : book_tags.GetTags()){
            work_.exec_params(R"(INSERT INTO book_tags (book_id, tag)
                     VALUES ($1, $2);)"_zv, book_tags.GetBookId(), tag);
        }
    }

    std::vector<std::string> BookTagsRepositoryImpl::ReadById(const std::string &book_id) {
        std::vector<std::string> tags;
        auto query_text = "SELECT tag FROM book_tags WHERE book_id='" + book_id;
        query_text += "' LIMIT 1;"_zv;
        std::optional result_tag = work_.query01<std::string>(query_text);
        if (result_tag) {
            auto query_text_all = "SELECT tag FROM book_tags WHERE book_id='" + book_id + "';";
            for (auto [tag] : work_.query<std::string>(query_text_all)) {
                tags.push_back(tag);
            }
            /*auto [str] = *result_tag;
            tags = split(str, " ");  */
        }
        return tags;
    }

    void BookTagsRepositoryImpl::DeleteById(const std::string &book_id) {
        auto query_text = "DELETE FROM book_tags WHERE book_id='" + book_id;
        query_text += "';"_zv;
        work_.exec(query_text);
    }

    Database::Database(pqxx::connection connection) : connection_{std::move(connection)} {
        pqxx::work work{connection_};
        /*work.exec(R"(CREATE TABLE IF NOT EXISTS authors (id UUID CONSTRAINT author_id_constraint PRIMARY KEY,
        name varchar(100) UNIQUE NOT NULL);)"_zv);*/
        work.exec(R"(CREATE TABLE IF NOT EXISTS authors (id UUID PRIMARY KEY,
        name varchar(100) UNIQUE NOT NULL);)"_zv);

        // ... создать другие таблицы
        /*work.exec(R"(CREATE TABLE IF NOT EXISTS books (id UUID CONSTRAINT book_id_constraint PRIMARY KEY,
        author_id UUID NOT NULL, title varchar(100) NOT NULL, publication_year integer);)"_zv);*/
        work.exec(R"(CREATE TABLE IF NOT EXISTS books (id UUID PRIMARY KEY, author_id UUID REFERENCES authors(id) NOT NULL,
        title varchar(100) NOT NULL, publication_year integer);)"_zv);

        work.exec(R"(CREATE TABLE IF NOT EXISTS book_tags (book_id UUID REFERENCES books(id) NOT NULL, tag varchar(30) NOT NULL);)"_zv);

        //Отключил очистку данных в таблицах для прохождения тестов в ../../../tests/test_s04_bookypedia-1.py
        //work.exec("DELETE FROM book_tags;"_zv); //Delete book_tags data
        //work.exec("DELETE FROM books;"_zv);     //Delete books data
        //work.exec("DELETE FROM authors;"_zv);   //Delete authors data
        // коммитим изменения
        work.commit();
    }


}  // namespace postgres