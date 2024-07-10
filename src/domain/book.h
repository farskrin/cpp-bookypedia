#pragma once
#include <string>
#include <vector>

#include "../util/tagged_uuid.h"
#include "author.h"

namespace domain {

    namespace detail {
        struct BookTag {};
    }  // namespace detail

    using BookId = util::TaggedUUID<detail::BookTag>;

    class Book {
    public:
        Book(BookId id, std::string author_id, std::string title, int year)
                : id_(std::move(id))
                , author_id_(std::move(author_id))
                , title_(std::move(title))
                , year_(year){
        }

        const BookId& GetId() const noexcept {
            return id_;
        }

        const std::string& GetAuthorId() const noexcept {
            return author_id_;
        }

        const std::string& GetTitle() const noexcept {
            return title_;
        }

        const int GetYear() const noexcept {
            return year_;
        }

    private:
        BookId id_;
        std::string author_id_;
        std::string title_;
        int year_ = 0;
    };

    struct BookData{
        std::string id;
        std::string author_id;
        std::string author_name;
        std::string title;
        int year = 0;
    };

    class BookRepository {
    public:
        virtual void Save(const Book& book) = 0;
        virtual std::vector<domain::BookData> Read() = 0;
        virtual std::vector<domain::BookData> ReadByName(const std::string& book_name) = 0;
        virtual domain::BookData ReadById(const std::string& book_id) = 0;
        virtual std::vector<domain::BookData> ReadAuthorBooks(const std::string& author_id) = 0;

        virtual void DeleteByName(const std::string& book_name ) = 0;
        virtual void DeleteById(const std::string& book_id ) = 0;

        virtual void EditTitleById(const std::string& id, const std::string& new_name) = 0;
        virtual void EditYearById(const std::string& id, int new_year) = 0;


    protected:
        ~BookRepository() = default;
    };

    class BookTags{
    public:
        BookTags(const std::string& book_id, const std::vector<std::string>& tags)
            : book_id_(book_id), tags_(tags){
        }

        const std::string& GetBookId() const noexcept {
            return book_id_;
        }

        const std::vector<std::string>& GetTags() const noexcept {
            return tags_;
        }

    private:
        std::string book_id_;
        std::vector<std::string> tags_;
    };

    class BookTagsRepository{
    public:
        virtual void Save(const BookTags& book_tags) = 0;
        virtual std::vector<std::pair<std::string, std::string>> Read() = 0;
        virtual std::vector<std::string> ReadById(const std::string& book_id) = 0;
        virtual void Update(const BookTags& book_tags) = 0;
        virtual void DeleteById(const std::string& book_id) = 0;

    protected:
        ~BookTagsRepository() = default;
    };

}  // namespace domain
