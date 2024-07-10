#include <catch2/catch_test_macros.hpp>

#include "../src/app/use_cases_impl.h"
#include "../src/domain/author.h"
#include "../src/domain/book.h"

namespace {

struct MockAuthorRepository : domain::AuthorRepository {
    std::vector<domain::Author> saved_authors;

    void Save(const domain::Author& author) override {
        saved_authors.emplace_back(author);
    }
    std::vector<std::pair<std::string, std::string>> Read() {
        return std::vector<std::pair<std::string, std::string>>();
    }
};

struct MockBookRepository : domain::BookRepository {
    std::vector<domain::Book> saved_books;

    void Save(const domain::Book& book) override {
        saved_books.emplace_back(book);
    }
    std::vector<domain::BookData> Read() {
        return  std::vector<domain::BookData>();
    }
    std::vector<domain::BookData> ReadAuthorBooks(const std::string& author_id) {
        return std::vector<domain::BookData>();
    }
};

struct MockBookTagsRepository : domain::BookTagsRepository {
    std::vector<domain::BookTags> saved_book_tags;

    void Save(const domain::BookTags& book_tags) override {
        saved_book_tags.emplace_back(book_tags);
    }
    std::vector<std::pair<std::string, std::string>> Read() {
        return  std::vector<std::pair<std::string, std::string>>();
    }
};

struct MockUnitOfWorkImpl : app::UnitOfWork{
    MockAuthorRepository* Author();
    MockBookRepository* Book();
    MockBookTagsRepository* BookTags();
   void Commit();
};

struct MockUnitOfWorkFactoryImpl : public app::UnitOfWorkFactory {

    app::UnitOfWork* CreateUnitOfWork() override{
        return new MockUnitOfWorkImpl();
    }
};

struct Fixture {
    MockUnitOfWorkFactoryImpl factory;
};

}  // namespace

SCENARIO_METHOD(Fixture, "Book Adding") {
    GIVEN("Use cases") {
        app::UseCasesImpl use_cases{factory};

        WHEN("Adding an author") {
            const auto author_name = "Joanne Rowling";
            use_cases.AddAuthor(author_name);

            /*THEN("author with the specified name is saved to repository") {
                REQUIRE(authors.saved_authors.size() == 1);
                CHECK(authors.saved_authors.at(0).GetName() == author_name);
                CHECK(authors.saved_authors.at(0).GetId() != domain::AuthorId{});
            }*/
        }
    }
}