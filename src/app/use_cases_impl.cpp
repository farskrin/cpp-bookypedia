#include "use_cases_impl.h"
#include <stdexcept>

#include "../domain/author.h"
#include "../domain/book.h"

namespace app {
    using namespace domain;

    std::string UseCasesImpl::AddAuthor(const std::string& name) {
        auto author_id = AuthorId::New();
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        try{
            unit->Author()->Save({author_id, name});
            unit->Commit();
            return author_id.ToString();
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed AddAuthor");
        }
    }

    std::vector<std::pair<std::string, std::string>> UseCasesImpl::ShowAuthors() {
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        try{
            std::vector<std::pair<std::string, std::string>> result = unit->Author()->Read();
            unit->Commit();
            return result;
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed ShowAuthors");
        }
    }

    void UseCasesImpl::DeleteAuthorByName(const std::string &name) {
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        try{
            unit->Author()->DeleteByName(name);
            unit->Commit();
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed DeleteAuthorByName");
        }
    }

    void UseCasesImpl::DeleteAuthorById(const std::string &id) {
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        try{
            unit->Author()->DeleteById(id);
            unit->Commit();
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed DeleteAuthorById");
        }
    }

    std::string UseCasesImpl::AddBook(const std::string &author_id, const std::string &title, int year) {
        auto book_id = BookId::New();
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        try{
            unit->Book()->Save( {book_id, author_id, title, year} );
            unit->Commit();
            return book_id.ToString();
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed AddBook");
        }
    }

    void UseCasesImpl::AddBookTags(const std::string& book_id, const std::vector<std::string>& tags) {
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        try{
            unit->BookTags()->Save(BookTags{book_id, tags});
            unit->Commit();
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed AddBookTags");
        }
    }

    std::vector<BookData> UseCasesImpl::ShowBooks() {
        /*struct BookData{
            std::string id;
            std::string author_id;
            std::string author_name;
            std::string title;
            int year = 0;
        };*/
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        std::vector<BookData> book_data;
        try{
            for(const auto& [id, author_id, author_name, title, year]
                                                : unit->Book()->Read()){
                book_data.push_back({id, author_id, author_name, title, year});
            }
            unit->Commit();
            return book_data;
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed ShowBooks");
        }
    }

    std::vector<BookData> UseCasesImpl::ShowBooksByTitle(const std::string &book_title) {
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        std::vector<BookData> book_data;
        try{
            for(const auto& [id, author_id, author_name, title, year]
                                                : unit->Book()->ReadByName(book_title)){
                book_data.push_back({id, author_id, author_name, title, year});
            }
            unit->Commit();
            return book_data;
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed ShowBooksByTitle");
        }
    }

    ShowBookData UseCasesImpl::ShowBookById(const std::string &book_id) {
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        ShowBookData show_book;
        try{
            auto book_data = unit->Book()->ReadById(book_id);
            show_book.title = book_data.title;
            show_book.author_name = book_data.author_name;
            show_book.publication_year = book_data.year;
            show_book.tags = unit->BookTags()->ReadById(book_id);
            unit->Commit();
            return show_book;
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed ShowBookById");
        }
    }

    std::vector<BookData> UseCasesImpl::ShowAuthorBooks(const std::string &a_id) {
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        std::vector<BookData> book_data;
        try{
            for(const auto& [id, author_id, author_name, title, year]
                                                : unit->Book()->ReadAuthorBooks(a_id)){
                book_data.push_back({id, author_id, author_name, title, year});
            }
            unit->Commit();
            return book_data;
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed ShowAuthorBooks");
        }
    }

    /*void UseCasesImpl::Commit() {
        authors_.Commit();
        books_.Commit();
        book_tags_.Commit();
    }*/

    void UseCasesImpl::EditAuthorByName(const std::string &old_name, const std::string &new_name) {
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        try{
            unit->Author()->EditByName(old_name, new_name);
            unit->Commit();
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed EditAuthorByName");
        }
    }

    void UseCasesImpl::EditAuthorById(const std::string &id, const std::string &new_name) {
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        try{
            unit->Author()->EditById(id, new_name);
            unit->Commit();
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed EditAuthorById");
        }
    }

    void UseCasesImpl::DeleteBookByName(const std::string &name) {
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        try{
            unit->Book()->DeleteByName(name);
            unit->Commit();
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed DeleteBookByName");
        }
    }

    void UseCasesImpl::DeleteBookById(const std::string &id) {
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        try{
            unit->Book()->DeleteById(id);
            unit->Commit();
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed DeleteBookById");
        }
    }

    void UseCasesImpl::EditBookTitleById(const std::string &id, const std::string &new_name) {
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        try{
            unit->Book()->EditTitleById(id, new_name);
            unit->Commit();
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed EditBookTitleById");
        }
    }

    void UseCasesImpl::EditBookYearById(const std::string &id, int new_year) {
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        try{
            unit->Book()->EditYearById(id, new_year);
            unit->Commit();
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed EditBookYearById");
        }
    }

    std::vector<std::string> UseCasesImpl::GetBookTagsById(const std::string &book_id) {
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        std::vector<std::string> book_tags;
        try{
            for(const auto& tag : unit->BookTags()->ReadById(book_id)){
                book_tags.push_back(tag);
            }
            unit->Commit();
            return book_tags;
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed GetBookTagsById");
        }
    }

    void UseCasesImpl::DeleteBookTagsById(const std::string &book_id) {
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        try{
            unit->BookTags()->DeleteById(book_id);
            unit->Commit();
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed DeleteBookTagsById");
        }
    }

    void UseCasesImpl::EditBookTagsById(const std::string &id, const std::vector<std::string> &new_tags) {
        auto unit = unit_of_work_factory_.CreateUnitOfWork();
        try{
            unit->BookTags()->Update({id, new_tags});
            unit->Commit();
        } catch (const std::exception&) {
            unit->Commit();
            throw std::logic_error("Failed EditBookTagsById");
        }
    }

}  // namespace app
