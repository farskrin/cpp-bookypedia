#pragma once
#include "../domain/author_fwd.h"
#include "use_cases.h"
#include "unit_of_work.h"

namespace app {

    class UseCasesImpl : public UseCases {
    public:
        explicit UseCasesImpl(UnitOfWorkFactory& factory)
                : unit_of_work_factory_(factory){
        }

        std::string AddAuthor(const std::string& name) override;
        std::vector<std::pair<std::string, std::string>> ShowAuthors() override;
        void DeleteAuthorByName(const std::string& name) override;
        void DeleteAuthorById(const std::string& id) override;
        void EditAuthorByName(const std::string& old_name, const std::string& new_name) override;
        void EditAuthorById(const std::string& id, const std::string& new_name) override;

        std::string AddBook(const std::string& author_id, const std::string& title, int year ) override;
        void AddBookTags(const std::string& book_id, const std::vector<std::string>& tags) override;
        std::vector<BookData> ShowBooks() override;
        std::vector<BookData> ShowBooksByTitle(const std::string& title) override;
        ShowBookData ShowBookById(const std::string& book_id)  override;
        std::vector<BookData> ShowAuthorBooks(const std::string& author_id) override;
        void DeleteBookByName(const std::string& name) override;
        void DeleteBookById(const std::string& id) override;

        void EditBookTitleById(const std::string& id, const std::string& new_name) override;
        void EditBookYearById(const std::string& id, int new_year) override;

        std::vector<std::string> GetBookTagsById(const std::string& book_id) override;
        void DeleteBookTagsById(const std::string& book_id) override;
        void EditBookTagsById(const std::string& id, const std::vector<std::string>& new_tags) override;

        //void Commit() override;
    private:

        /*domain::AuthorRepository& authors_;
        domain::BookRepository& books_;
        domain::BookTagsRepository& book_tags_;*/

        UnitOfWorkFactory& unit_of_work_factory_;   //TODO: add impl_
    };

}  // namespace app
