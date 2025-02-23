#include <concepts>
#include <memory>
#include <utility>

namespace SimpleMessageQueue {

template<typename T, typename K, typename P>
concept RequestObjectConcept = requires(T obj) {
    { obj.GetKey() } -> std::convertible_to<K>;
    { obj.GetPayload() } -> std::convertible_to<P>;
};

template<typename Key, typename Payload>
class Request {
public:
    template<RequestObjectConcept<Key, Payload> T>
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

template<typename T, typename P>
concept ResponseObjectConcept = requires(T obj) {
    { obj.GetPayload() } -> std::convertible_to<P>;
};

template<typename Payload>
class Response {
public:
    Response(Payload payload)
        : object(std::make_shared<Model<RawPayloadWrapper>>(payload)) {}

    template<ResponseObjectConcept<Payload> T>
    Response(T&& t)
        : object(std::make_shared<Model<T>>(std::forward<T>(t))) {}

    Payload GetPayload() const { return object->GetPayload(); }

private:
    Response() = delete;

    struct RawPayloadWrapper {
        RawPayloadWrapper(Payload payload)
            : value(payload) {}
        Payload GetPayload() const { return value; }
        Payload value;
    };

    struct Concept {
        virtual ~Concept()                 = default;
        virtual Payload GetPayload() const = 0;
    };

    template<ResponseObjectConcept<Payload> T>
    struct Model : Concept {
        Model(T&& t)
            : object(std::forward<T>(t)) {}

        Payload GetPayload() const override { return object.GetPayload(); }

    private:
        T object;
    };

    std::shared_ptr<const Concept> object;
};

} // namespace SimpleMessageQueue
