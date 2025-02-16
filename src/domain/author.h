#pragma once
#include <string>
#include <vector>

#include "../util/tagged_uuid.h"

namespace domain {

namespace detail {
struct AuthorTag {};
}  // namespace detail

using AuthorId = util::TaggedUUID<detail::AuthorTag>;

class Author {
public:
    Author(AuthorId id, std::string name)
        : id_(std::move(id))
        , name_(std::move(name)) {
    }

    const AuthorId& GetId() const noexcept {
        return id_;
    }

    const std::string& GetName() const noexcept {
        return name_;
    }

private:
    AuthorId id_;
    std::string name_;
};

class AuthorRepository {
public:
    virtual void Save(const Author& author) = 0;
    virtual std::vector<std::pair<std::string, std::string>> Read() = 0;
    virtual void DeleteByName(const std::string& name) = 0;
    virtual void DeleteById(const std::string& id) = 0;
    virtual void EditByName(const std::string& old_name, const std::string& new_name) = 0;
    virtual void EditById(const std::string& id, const std::string& new_name) = 0;

protected:
    ~AuthorRepository() = default;
};

}  // namespace domain
