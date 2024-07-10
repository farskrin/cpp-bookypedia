#pragma once

#include <string>
#include <vector>

namespace app {

    struct BookData{
        std::string id;
        std::string author_id;
        std::string author_name;
        std::string title;
        int year = 0;
    };

    struct ShowBookData {
        std::string title;
        std::string author_name;
        int publication_year;
        std::vector<std::string> tags;
    };

class UseCases {
public:
    //virtual void CreateUnitOfWork() = 0;

    virtual std::string AddAuthor(const std::string& name) = 0;
    virtual std::vector<std::pair<std::string, std::string>> ShowAuthors() = 0;
    virtual void DeleteAuthorByName(const std::string& name) = 0;
    virtual void DeleteAuthorById(const std::string& id) = 0;
    virtual void EditAuthorByName(const std::string& old_name, const std::string& new_name) = 0;
    virtual void EditAuthorById(const std::string& id, const std::string& new_name) = 0;

    virtual std::string AddBook(const std::string& author_id, const std::string& title, int year ) = 0;
    virtual void AddBookTags(const std::string& book_id, const std::vector<std::string>& tags) = 0;
    virtual std::vector<BookData> ShowBooks() = 0;
    virtual std::vector<BookData> ShowBooksByTitle(const std::string& title) = 0;
    virtual ShowBookData ShowBookById(const std::string& book_id) = 0;
    virtual std::vector<BookData> ShowAuthorBooks(const std::string& author_id) = 0;
    virtual void DeleteBookByName(const std::string& name) = 0;
    virtual void DeleteBookById(const std::string& id) = 0;
    virtual void EditBookTitleById(const std::string& id, const std::string& new_name) = 0;
    virtual void EditBookYearById(const std::string& id, int new_year) = 0;

    virtual std::vector<std::string> GetBookTagsById(const std::string& book_id) = 0;
    virtual void DeleteBookTagsById(const std::string& book_id) = 0;
    virtual void EditBookTagsById(const std::string& id, const std::vector<std::string>& new_tags) = 0;

    //virtual void Commit() = 0;

protected:
    ~UseCases() = default;
};

}  // namespace app
