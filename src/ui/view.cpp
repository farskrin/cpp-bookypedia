#include "view.h"

#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string.hpp>
#include <cassert>
#include <iostream>
#include <algorithm>
#include <sstream>

#include "../app/use_cases.h"
#include "../menu/menu.h"

using namespace std::literals;
namespace ph = std::placeholders;

namespace ui {
namespace detail {

std::ostream& operator<<(std::ostream& out, const AuthorInfo& author) {
    out << author.name;
    return out;
}

std::ostream& operator<<(std::ostream& out, const BookInfo& book) {
    if(!book.author_name.empty()){
        out << book.title << " by " << book.author_name << ", " << book.publication_year;
    }
    else
    {
        out << book.title << ", " << book.publication_year;     //TODO: check condition ???
    }

    return out;
}

std::vector<std::string> ParseTags(std::string tags_str){
    std::string::iterator it =
            std::unique(tags_str.begin(), tags_str.end(),
                        [] (char lt, char rg) {return lt == ' ' && rg == ' ';});
    tags_str.erase(it, tags_str.end());

    std::set<std::string> tags_set;
    boost::algorithm::split(tags_set, tags_str, boost::is_any_of(","));

    std::vector<std::string> tags;
    tags.reserve(tags_set.size());
    for(auto tag : tags_set){
        boost::algorithm::trim(tag);
        if(!tag.empty()){
            tags.push_back(tag);
        }
    }
    tags_set.clear();
    tags_set.insert(tags.begin(), tags.end());
    tags.clear();
    tags.insert(tags.end(), tags_set.begin(), tags_set.end());

    return tags;
}

}  // namespace detail

template <typename T>
void PrintVector(std::ostream& out, const std::vector<T>& vector) {
    int i = 1;
    for (auto& value : vector) {
        out << i++ << " " << value << std::endl;
    }
}

View::View(menu::Menu& menu, app::UseCases& use_cases, std::istream& input, std::ostream& output)
    : menu_{menu}
    , use_cases_{use_cases}
    , input_{input}
    , output_{output} {
    menu_.AddAction(  //
        "AddAuthor"s, "name"s, "Adds author"s, std::bind(&View::AddAuthor, this, ph::_1)
        // либо
        // [this](auto& cmd_input) { return AddAuthor(cmd_input); }
    );
    menu_.AddAction("DeleteAuthor"s, "name"s, "Delete authors"s,
                    std::bind(&View::DeleteAuthor, this, ph::_1));
    menu_.AddAction("EditAuthor"s, "name"s, "Edit authors"s,
                    std::bind(&View::EditAuthor, this, ph::_1));

    menu_.AddAction("AddBook"s, "<pub year> <title>"s, "Adds book"s,
                    std::bind(&View::AddBook, this, ph::_1));
    menu_.AddAction("ShowAuthors"s, {}, "Show authors"s, std::bind(&View::ShowAuthors, this));
    menu_.AddAction("ShowBooks"s, {}, "Show books"s, std::bind(&View::ShowBooks, this));
    menu_.AddAction("ShowAuthorBooks"s, {}, "Show author books"s,
                    std::bind(&View::ShowAuthorBooks, this));
    menu_.AddAction("ShowBook"s, "name"s, "Show book"s,
                    std::bind(&View::ShowBook, this, ph::_1));
    menu_.AddAction("DeleteBook"s, "name"s, "Delet book"s,
                    std::bind(&View::DeleteBook, this, ph::_1));
    menu_.AddAction("EditBook"s, "name"s, "Edit book"s,
                    std::bind(&View::EditBook, this, ph::_1));
}

bool View::AddAuthor(std::istream& cmd_input) const {
    try {
        std::string name;
        std::getline(cmd_input, name);
        boost::algorithm::trim(name);
        if(name.empty()){
            throw std::logic_error("AddAuthor: name.empty()");
        }
        use_cases_.AddAuthor(std::move(name));
    } catch (const std::exception&) {
        output_ << "Failed to add author"sv << std::endl;
    }
    return true;
}

bool View::DeleteAuthor(std::istream &cmd_input) {

    //TODO:!!! delete relation tables books and book_tags
    try {
        std::string author_name;
        std::getline(cmd_input, author_name);
        boost::algorithm::trim(author_name);
        if(!author_name.empty()){
            use_cases_.DeleteAuthorByName(author_name);
            return true;
        }
        //-----
        if (auto author_id = SelectAuthor()) {
            use_cases_.DeleteAuthorById(*author_id);
        }
    } catch (const std::exception& e) {
        //std::cout << e.what() << std::endl;     //TODO: delete this
        //throw std::runtime_error("Failed to delete author");
        output_ << "Failed to delete author"sv << std::endl;
    }
    return true;
}

bool View::EditAuthor(std::istream &cmd_input) {
    try{
        std::string old_name;
        std::getline(cmd_input, old_name);
        boost::algorithm::trim(old_name);
        if(!old_name.empty()){
            output_ << "Enter new name:"sv << std::endl;
            std::string new_name;
            std::getline(input_, new_name);
            boost::algorithm::trim(new_name);
            use_cases_.EditAuthorByName(old_name, new_name);

            return true;
        }
        //-----
        if (auto author_id = SelectAuthor()) {
            output_ << "Enter new name:"sv << std::endl;
            std::string new_name;
            std::getline(input_, new_name);
            boost::algorithm::trim(new_name);
            use_cases_.EditAuthorById(*author_id, new_name);

        }
    } catch (const std::exception& e) {
        //std::cout << e.what() << std::endl;     //TODO: delete this
        //throw std::runtime_error("Failed to edit author");
        output_ << "Failed to edit author"sv << std::endl;
    }
    return true;
}

bool View::AddBook(std::istream& cmd_input) const {
    try {
        if (auto params = GetBookParams(cmd_input)) {
            //assert(!"TODO: implement book adding");
            std::string book_id = use_cases_.AddBook(params->author_id, params->title, params->publication_year);
            //TODO: Create book_tag
            if(!params->tags.empty()){
                use_cases_.AddBookTags(book_id, params->tags);
            }
        }
    } catch (const std::exception& e) {
        //std::cout << e.what() << std::endl; //TODO: delete this
        output_ << "Failed to add book"sv << std::endl;
    }
    return true;
}

bool View::ShowAuthors() const {
    PrintVector(output_, GetAuthors());
    return true;
}

bool View::ShowBooks() const {
    PrintVector(output_, GetBooks());
    return true;
}

bool View::ShowBook(std::istream& cmd_input) {

    try {
        std::string book_name;
        std::getline(cmd_input, book_name);
        boost::algorithm::trim(book_name);
        if(!book_name.empty()){

            std::vector<app::BookData> book_datas = use_cases_.ShowBooksByTitle(book_name);
            if(book_datas.empty()){
                return true;
            } else if(book_datas.size() > 1) {
                //TODO: Choose book by id
                //SelectBookByName
                if (auto book_id = SelectBookByName(book_name)) {

                    app::ShowBookData show_data = use_cases_.ShowBookById(*book_id);
                    //Print:
                    output_ << "Title: " << show_data.title << std::endl;
                    output_ << "Author: " << show_data.author_name << std::endl;
                    output_ << "Publication year: " << show_data.publication_year << std::endl;
                    if(!show_data.tags.empty()){
                        output_ << "Tags: ";
                    }
                    bool first = true;
                    for(const auto& tag : show_data.tags){
                        if (!first) {
                            output_ << ", "s;
                        }
                        first = false;
                        output_ << tag;
                    }
                    if(!show_data.tags.empty()){
                        output_ << std::endl;
                    }

                }

            } else {    //Equal one book
                //Print:
                output_ << "Title: " << book_datas.front().title << std::endl;
                output_ << "Author: " << book_datas.front().author_name << std::endl;
                output_ << "Publication year: " << book_datas.front().year << std::endl;
                std::vector<std::string> tags = use_cases_.GetBookTagsById(book_datas.front().id);
                if(!tags.empty()){
                    output_ << "Tags: ";
                }
                bool first = true;
                for(const auto& tag : tags){
                    if (!first) {
                        output_ << ", "s;
                    }
                    first = false;
                    output_ << tag;
                }
                if(!tags.empty()){
                    output_ << std::endl;
                }
            }
            return true;
        }
        //-----
        if (auto book_id = SelectBook()) {
            app::ShowBookData show_data = use_cases_.ShowBookById(*book_id);
            //Print:
            output_ << "Title: " << show_data.title << std::endl;
            output_ << "Author: " << show_data.author_name << std::endl;
            output_ << "Publication year: " << show_data.publication_year << std::endl;
            if(!show_data.tags.empty()){
                output_ << "Tags: ";
            }
            bool first = true;
            for(const auto& tag : show_data.tags){
                if (!first) {
                    output_ << ", "s;
                }
                first = false;
                output_ << tag;
            }
            if(!show_data.tags.empty()){
                output_ << std::endl;
            }
        }

    } catch (const std::exception& e) {
        //std::cout << e.what() << std::endl;     //TODO: delete this
        //throw std::runtime_error("Failed to show book");
        output_ << "Failed to show book"sv << std::endl;
    }
    return true;
}

bool View::ShowAuthorBooks() const {
    try {
        if (auto author_id = SelectAuthor()) {
            PrintVector(output_, GetAuthorBooks(*author_id));
        }
    } catch (const std::exception& e) {
        //std::cout << e.what() << std::endl;
        //throw std::runtime_error("Failed to Show Books");
        output_ << "Failed to Show Books"sv << std::endl;
    }
    return true;
}

std::optional<detail::AddBookParams> View::GetBookParams(std::istream& cmd_input) const {
    detail::AddBookParams params;

    cmd_input >> params.publication_year;
    std::getline(cmd_input, params.title);
    boost::algorithm::trim(params.title);
    //---TODO: check this
    std::string author_name;
    output_ << "Enter author name or empty line to select from list:" << std::endl;
    std::getline(input_, author_name);
    boost::algorithm::trim(author_name);
    if(!author_name.empty()){
        auto authors = GetAuthors();
        auto it = std::find_if(authors.begin(), authors.end(),
                               [&author_name](const detail::AuthorInfo& author){
                                   return author_name == author.name;
                               });
        if(it==authors.end()){
            output_ << "No author found. Do you want to add " << author_name << " (y/n)?" << std::endl;
            std::string answer_yes;
            std::getline(input_, answer_yes);
            boost::to_lower(answer_yes);
            if(answer_yes != "y" ){
                throw std::logic_error("GetBookParams: answer_yes != yes");
            }
            params.author_id =  use_cases_.AddAuthor(author_name);  //TODO: add author without commit
            //TODO: add tags
            output_ << "Enter tags (comma separated):" << std::endl;
            std::string tags_str;
            std::getline(input_, tags_str);
            std::vector<std::string> tags;
            if(!tags_str.empty()){
                params.tags = detail::ParseTags(tags_str);
            }
            return params;
        }
    }
    //---SelectAuthor()---
    auto author_id = SelectAuthor();
    if (not author_id.has_value())
        return std::nullopt;
    else {
        params.author_id = author_id.value();
        //TODO: add tags
        output_ << "Enter tags (comma separated):" << std::endl;
        std::string tags_str;
        std::getline(input_, tags_str);
        std::vector<std::string> tags;
        if(!tags_str.empty()){
            params.tags = detail::ParseTags(tags_str);
        }
        return params;
    }
}


//TODO: SelectAuthor()
std::optional<std::string> View::SelectAuthor() const {

    output_ << "Select author:" << std::endl;
    auto authors = GetAuthors();
    PrintVector(output_, authors);
    output_ << "Enter author # or empty line to cancel" << std::endl;

    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }

    int author_idx;
    try {
        author_idx = std::stoi(str);
    } catch (std::exception const&) {
        throw std::runtime_error("Invalid author num");
    }

    --author_idx;
    if (author_idx < 0 or author_idx >= authors.size()) {
        throw std::runtime_error("Invalid author num");
    }

    return authors[author_idx].id;
}

//TODO: SelectBook()
std::optional<std::string> View::SelectBook() const {

    auto book_info = GetBooks();
    PrintVector(output_, book_info);
    output_ << "Enter the book # or empty line to cancel:" << std::endl;
    std::string str;
    if (!std::getline(input_, str) || str.empty()) {
        return std::nullopt;
    }

    int book_idx;
    try {
        book_idx = std::stoi(str);
    } catch (std::exception const&) {
        throw std::runtime_error("Invalid book num");
    }

    --book_idx;
    if (book_idx < 0 or book_idx >= book_info.size()) {
        throw std::runtime_error("Invalid book num");
    }

    return book_info[book_idx].id;
}

//TODO: SelectBookByName()
std::optional<std::string> View::SelectBookByName(const std::string &title) const {

        auto book_info = GetBooksByName(title);
        PrintVector(output_, book_info);
        output_ << "Enter the book # or empty line to cancel:" << std::endl;
        std::string str;
        if (!std::getline(input_, str) || str.empty()) {
            return std::nullopt;
        }

        int book_idx;
        try {
            book_idx = std::stoi(str);
        } catch (std::exception const&) {
            throw std::runtime_error("Invalid book num");
        }

        --book_idx;
        if (book_idx < 0 or book_idx >= book_info.size()) {
            throw std::runtime_error("Invalid book num");
        }
        return book_info[book_idx].id;
}

std::vector<detail::AuthorInfo> View::GetAuthors() const {
    std::vector<detail::AuthorInfo> dst_autors;
    //assert(!"TODO: implement GetAuthors()");
    //"SELECT id, name FROM authors ORDER BY name;"
    for(const auto& [id, name] : use_cases_.ShowAuthors()){
        dst_autors.push_back({id, name});
    }

    return dst_autors;
}

std::vector<detail::BookInfo> View::GetBooks() const {
    std::vector<detail::BookInfo> books;
    //assert(!"TODO: implement GetBooks()");
    //"SELECT id, title, author_id, publication_year FROM books ORDER BY title;"
    for(const auto& [id, author_id, author_name, title, year]
                                                    : use_cases_.ShowBooks()){
        books.push_back({id, title, author_name, year});
    }

    return books;
}

std::vector<detail::BookInfo> View::GetBooksByName(const std::string &book_name) const {
    std::vector<detail::BookInfo> books;
    for(const auto& [id, author_id, author_name, title, year]
            : use_cases_.ShowBooksByTitle(book_name)){
        books.push_back({id, title, author_name, year});
    }
    return books;
}

std::vector<detail::BookInfo> View::GetAuthorBooks(const std::string& a_id) const {
    std::vector<detail::BookInfo> books;
    //assert(!"TODO: implement GetAuthorBooks()");
    //"SELECT id, title, author_id, publication_year FROM books WHERE author_id=%s ORDER BY publication_year, title;"
    for(const auto& [id, author_id, author_name, title, year]
                                                    : use_cases_.ShowAuthorBooks(a_id)){
        books.push_back({id, title, author_name, year});
    }

    return books;
}

    bool View::DeleteBook(std::istream &cmd_input) {

        try {
            std::string book_name;
            std::getline(cmd_input, book_name);
            boost::algorithm::trim(book_name);
            if(!book_name.empty()){

                std::vector<app::BookData> book_datas = use_cases_.ShowBooksByTitle(book_name);
                if(book_datas.empty()){
                    throw std::logic_error("DeleteBook: book not exist");
                    return false;
                } else if(book_datas.size() > 1) {
                    //TODO: Choose book by id
                    //SelectBookByName
                    if (auto book_id = SelectBookByName(book_name)) {
                        std::vector<std::string> tags = use_cases_.GetBookTagsById(*book_id);
                        if(!tags.empty()){
                            use_cases_.DeleteBookTagsById(*book_id);
                        }
                        use_cases_.DeleteBookById(*book_id);
                    }
                } else {    //Equal one book
                    std::vector<std::string> tags = use_cases_.GetBookTagsById(book_datas.front().id);
                    if(!tags.empty()){
                        use_cases_.DeleteBookTagsById(book_datas.front().id);
                    }
                    use_cases_.DeleteBookByName(book_name);
                }
                return true;
            }
            //-----
            if (auto book_id = SelectBook()) {
                std::vector<std::string> tags = use_cases_.GetBookTagsById(*book_id);
                if(!tags.empty()){
                    use_cases_.DeleteBookTagsById(*book_id);
                }
                use_cases_.DeleteBookById(*book_id);
            }
        } catch (const std::exception& e) {
            //std::cout << e.what() << std::endl;     //TODO: delete this
            //throw std::runtime_error("Failed to delete book");
            //output_ << "Failed to delete book"sv << std::endl;    //TODO: В задании указано так
            output_ << "Book not found"sv << std::endl;             //TODO: А тест ожидает так
        }
        return true;
    }

    bool View::EditBook(std::istream &cmd_input) {

        try {
            std::optional<std::string> new_title_opt;
            std::optional<int> new_year_opt;
            //-------
            std::string book_name;
            std::getline(cmd_input, book_name);
            boost::algorithm::trim(book_name);
            if(!book_name.empty()){

                std::vector<app::BookData> book_datas = use_cases_.ShowBooksByTitle(book_name);
                if(book_datas.empty()){
                    throw std::logic_error("EditBook: book not exist");
                    return false;
                } else if(book_datas.size() > 1) {
                    //TODO: Choose book by id
                    //SelectBookByName
                    if (auto book_id = SelectBookByName(book_name)) {

                        app::ShowBookData show_data = use_cases_.ShowBookById(*book_id);
                        //Title
                        output_ << "Enter new title or empty line to use the current one (" << show_data.title <<"):" << std::endl;
                        std::string new_title;
                        std::getline(input_, new_title);
                        boost::algorithm::trim(new_title);
                        if(!new_title.empty()){
                            new_title_opt = new_title;
                        }
                        //Year
                        output_ << "Enter publication year or empty line to use the current one (" << show_data.publication_year <<"):" << std::endl;
                        std::string new_year_str;
                        std::getline(input_, new_year_str);
                        boost::algorithm::trim(new_year_str);
                        if(!new_year_str.empty()){
                            int new_year = stoi(new_year_str);
                            new_year_opt = new_year;
                        }
                        //
                        output_ << "Enter tags (current tags: ";
                        bool first = true;
                        for(const auto& tag : show_data.tags){
                            if (!first) {
                                output_ << ", "s;
                            }
                            first = false;
                            output_ << tag;
                        }
                        output_ << "):" << std::endl;
                        //New tags
                        std::string tags_str;
                        std::getline(input_, tags_str);
                        std::vector<std::string> new_tags;

                        if(new_title_opt){
                            use_cases_.EditBookTitleById(*book_id, *new_title_opt);
                        }
                        if(new_year_opt){
                            use_cases_.EditBookYearById(*book_id, *new_year_opt);
                        }
                        new_tags = detail::ParseTags(tags_str);
                        use_cases_.EditBookTagsById(*book_id, new_tags);


                    }
                    else
                    {
                        throw std::logic_error("SelectBookByName has std::nullopt");
                    }

                } else {    //Equal one book
                    //Print:
                    //Title
                    output_ << "Enter new title or empty line to use the current one (" << book_datas.front().title <<"):" << std::endl;
                    std::string new_title;
                    std::getline(input_, new_title);
                    boost::algorithm::trim(new_title);
                    if(!new_title.empty()){
                        new_title_opt = new_title;
                    }
                    //Year
                    output_ << "Enter publication year or empty line to use the current one (" << book_datas.front().year <<"):" << std::endl;
                    std::string new_year_str;
                    std::getline(input_, new_year_str);
                    boost::algorithm::trim(new_year_str);
                    if(!new_year_str.empty()){
                        int new_year = stoi(new_year_str);
                        new_year_opt = new_year;
                    }
                    //
                    std::vector<std::string> tags = use_cases_.GetBookTagsById(book_datas.front().id);
                    output_ << "Enter tags (current tags: ";
                    bool first = true;
                    for(const auto& tag : tags){
                        if (!first) {
                            output_ << ", "s;
                        }
                        first = false;
                        output_ << tag;
                    }
                    output_ << "):" << std::endl;
                    //New tags
                    std::string tags_str;
                    std::getline(input_, tags_str);
                    std::vector<std::string> new_tags;

                    if(new_title_opt){
                        use_cases_.EditBookTitleById(book_datas.front().id, *new_title_opt);
                    }
                    if(new_year_opt){
                        use_cases_.EditBookYearById(book_datas.front().id, *new_year_opt);
                    }
                    new_tags = detail::ParseTags(tags_str);
                    use_cases_.EditBookTagsById(book_datas.front().id, new_tags);

                }
                return true;
            }
            //-----
            if (auto book_id = SelectBook()) {
                app::ShowBookData show_data = use_cases_.ShowBookById(*book_id);
                //Title
                output_ << "Enter new title or empty line to use the current one (" << show_data.title <<"):" << std::endl;
                std::string new_title;
                std::getline(input_, new_title);
                boost::algorithm::trim(new_title);
                if(!new_title.empty()){
                    new_title_opt = new_title;
                }
                //Year
                output_ << "Enter publication year or empty line to use the current one (" << show_data.publication_year <<"):" << std::endl;
                std::string new_year_str;
                std::getline(input_, new_year_str);
                boost::algorithm::trim(new_year_str);
                if(!new_year_str.empty()){
                    int new_year = stoi(new_year_str);
                    new_year_opt = new_year;
                }
                //
                output_ << "Enter tags (current tags: ";
                bool first = true;
                for(const auto& tag : show_data.tags){
                    if (!first) {
                        output_ << ", "s;
                    }
                    first = false;
                    output_ << tag;
                }
                output_ << "):" << std::endl;
                //New tags
                std::string tags_str;
                std::getline(input_, tags_str);
                std::vector<std::string> new_tags;

                if(new_title_opt){
                    use_cases_.EditBookTitleById(*book_id, *new_title_opt);
                }
                if(new_year_opt){
                    use_cases_.EditBookYearById(*book_id, *new_year_opt);
                }
                new_tags = detail::ParseTags(tags_str);
                use_cases_.EditBookTagsById(*book_id, new_tags);

            }
            else
            {
                throw std::logic_error("SelectBook has std::nullopt");
            }
        } catch (const std::exception& e) {
            //std::cout << e.what() << std::endl;     //TODO: delete this
            //throw std::runtime_error("Book not found");
            output_ << "Book not found"sv << std::endl;
        }
        return true;
    }


}  // namespace ui
