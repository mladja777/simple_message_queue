#include <memory>
#include <utility>

namespace SimpleMessageQueue {

template<typename Key, typename Payload>
class Request {
public:
    template<typename T>
    Request(T&& t)
        : object(std::make_shared<Model<T>>(std::forward<T>(t))) {}

    Key     GetKey() const { return object->GetKey(); }
    Payload GetPayload() const { return object->GetPayload(); }

private:
    Request() = delete;

    struct Concept {
        virtual ~Concept()                 = default;
        virtual Key     GetKey() const     = 0;
        virtual Payload GetPayload() const = 0;
    };

    template<typename T>
    struct Model : Concept {
        Model(T&& t)
            : object(std::forward<T>(t)) {}

        Key     GetKey() const override { return object.GetKey(); }
        Payload GetPayload() const override { return object.GetPayload(); }

    private:
        T object;
    };

    std::shared_ptr<const Concept> object;
};

} // namespace SimpleMessageQueue
