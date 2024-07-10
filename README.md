Справочник работает с DB PostgreSQL через клиент pqxx.

Модель (таблицы) в базе:
```cpp
struct AddBookParams {
    std::string title;
    std::string author_id;
    int publication_year = 0;
    std::vector<std::string> tags;
};
```
```cpp
struct AuthorInfo {
    std::string id;
    std::string name;
};
```
```cpp
struct BookInfo {
    std::string id;
    std::string title;
    std::string author_name;
    int publication_year;
};
```
