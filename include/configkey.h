#pragma once

#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qvariant.h>

namespace QJsonUtil {

    struct JsonReadInterface {
        //读取json的key
        virtual const QString& key() const = 0;
        //value转成QJsonValue
        virtual QJsonValue value() = 0;
        //写入值
        virtual void save(const QJsonValue& value) = 0;
        //字符串路由
        virtual JsonReadInterface* findByRouter(const QStringList& router) = 0;

        virtual ~JsonReadInterface() = default;
    };

    template<typename T>
    struct ConfigKey;

    struct JsonDumpInterface {

        //将JsonObject转换为模型字段
        virtual void fromJson(const QJsonObject &jsonObject) {
            for (const auto& item : prop()) {
                if (jsonObject.contains(item->key())) {
                    item->save(jsonObject.value(item->key()));
                }
            }
        }

        //将模型结构体转JsonObject
        virtual QJsonObject dumpToJson() {
            QJsonObject jsonObject;
            for (const auto& item : prop()) {
                jsonObject.insert(item->key(), item->value());
            }
            return jsonObject;
        }

        //读取模型类所有字段
        virtual QList<JsonReadInterface*> prop() = 0;

        //通过字符串路由查找模型字段
        template<typename T>
        ConfigKey<T>* findByRouter(const QString& router);

        JsonReadInterface* findByRouter(const QStringList& router);

        virtual ~JsonDumpInterface() = default;
    };

    template<typename T>
    struct JsonIdentity {
        using type = T;
    };

    template<typename I> struct IteratorType;
    template<typename I>
    struct IteratorType<QList<I>> {
        using type = I;
    };

    template<typename T>
    struct ConfigKey : JsonReadInterface {
        //保存json对应的key名称
        QString jsonKey;
        //保存存储的值
        T jsonValue;

        //初始化时将key传入
        explicit ConfigKey(QString key)
                : jsonKey(std::move(key)), jsonValue(T())
        {}

        //赋值
        ConfigKey& operator=(const T& v) {
            jsonValue = v;
            return *this;
        }

        //引用取值
        T& operator()() {
            return jsonValue;
        }

        //const取值
        const T& operator()() const {
            return jsonValue;
        }

        //读取key
        const QString& key() const override {
            return jsonKey;
        }

        template<typename K>
        using ValueType = typename std::conditional<std::is_base_of<JsonDumpInterface, K>::value, JsonDumpInterface, K>::type;

        //value转QJsonValue
        QJsonValue value() override {
            return toJsonValue(jsonValue, JsonIdentity<ValueType<T>>());
        }

        //保存
        void save(const QJsonValue &value) override {
            fromJsonValue(jsonValue, value, JsonIdentity<ValueType<T>>());
        }

        //通过字符串路由判断是否是自己
        JsonReadInterface* findByRouter(const QStringList& router) override {
            return findByRouter(router, JsonIdentity<ValueType<T>>());
        }

    private:
        //基础类型转换
        template<typename I, typename K>
        static QJsonValue toJsonValue(I& value, JsonIdentity<K>) {
            return value;
        }

        //ConfigKey类型
        template<typename I>
        static QJsonValue toJsonValue(I& value, JsonIdentity<JsonDumpInterface>) {
            return dynamic_cast<JsonDumpInterface*>(&value)->dumpToJson();
        }

        //容器类型
        template<typename I, typename K>
        static QJsonValue toJsonValue(I& value, JsonIdentity<QList<K>>) {
            QJsonArray jsonArray;
            for (auto& v : value) {
                jsonArray.append(toJsonValue(v, JsonIdentity<ValueType<K>>()));
            }
            return jsonArray;
        }

        //QStringList类型
        template<typename I>
        static QJsonValue toJsonValue(I& value, JsonIdentity<QStringList>) {
            return QJsonArray::fromStringList(value);
        }

        //基础类型赋值
        template<typename I, typename K>
        static void fromJsonValue(I& tagValue, const QJsonValue& value, JsonIdentity<K>) {
            tagValue = value.toVariant().value<K>();
        }

        //ConfigKey类型
        template<typename I>
        static void fromJsonValue(I& tagValue, const QJsonValue& value, JsonIdentity<JsonDumpInterface>) {
            dynamic_cast<JsonDumpInterface*>(&tagValue)->fromJson(value.toObject());
        }

        //容器类型
        template<typename I, typename K>
        static void fromJsonValue(I& tagValue, const QJsonValue& value, JsonIdentity<QList<K>>) {
            tagValue = QList<K>();
            auto values = value.toArray();
            for (const auto& v : values) {
                typename IteratorType<QList<K>>::type temp;
                fromJsonValue(temp, v, JsonIdentity<ValueType<K>>());
                tagValue.append(temp);
            }
        }

        template<typename K>
        JsonReadInterface* findByRouter(const QStringList& router, JsonIdentity<K>) {
            if (router.length() == 1 && router.first() == jsonKey) {
                return this;
            }
            return nullptr;
        }

        JsonReadInterface* findByRouter(const QStringList& router, JsonIdentity<JsonDumpInterface>) {
            return dynamic_cast<JsonDumpInterface*>(&jsonValue)->findByRouter(router);
        }

        template<typename K>
        JsonReadInterface* findByRouter(const QStringList& router, JsonIdentity<QList<K>>) {
            return findNextRouter(jsonValue, router, JsonIdentity<QList<K>>());
        }

        template<typename K>
        static JsonReadInterface* readerCaster(typename std::enable_if<std::is_base_of<JsonReadInterface, K>::value, K*>::type value) {
            return dynamic_cast<JsonReadInterface*>(value);
        }

        template<typename K>
        static JsonReadInterface* readerCaster(typename std::enable_if<!std::is_base_of<JsonReadInterface, K>::value, K*>::type) {
            return nullptr;
        }

        template<typename K>
        static JsonReadInterface* findNextRouter(typename std::enable_if<std::is_base_of<JsonDumpInterface, K>::value, K>::type& value, const QStringList& router) {
            return value.findByRouter(router);
        }

        template<typename K>
        static JsonReadInterface* findNextRouter(typename std::enable_if<!std::is_base_of<JsonDumpInterface, K>::value, K>::type& value, const QStringList& router) {
            return findNextRouter(value, router, JsonIdentity<K>());
        }

        template<typename K>
        static JsonReadInterface* findNextRouter(K&, const QStringList&, JsonIdentity<K>) {
            return nullptr;
        }

        template<typename K>
        static JsonReadInterface* findNextRouter(QList<K>& value, const QStringList& router, JsonIdentity<QList<K>>) {
            if (router.isEmpty()) {
                return nullptr;
            }

            bool ok;
            int arrayIndex = router.first().toInt(&ok);
            if (!ok) {
                return nullptr;
            }

            if (arrayIndex < 0 || arrayIndex >= value.size()) {
                return nullptr;
            }

            if (router.length() == 1) {
                return readerCaster<K>(&value[arrayIndex]);
            }
            return findNextRouter<K>(value[arrayIndex], router.mid(1));
        }
    };

    template<typename T>
    inline ConfigKey<T> *JsonDumpInterface::findByRouter(const QString &router) {
        auto routerList = router.split(".");
        if (routerList.isEmpty()) {
            return nullptr;
        }
        return dynamic_cast<ConfigKey<T>*>(findByRouter(routerList));
    }

    inline JsonReadInterface *JsonDumpInterface::findByRouter(const QStringList &router) {
        auto& key = router.first();
        for (auto item : prop()) {
            if (item->key() == key) {
                if (router.size() == 1) {
                    return item;
                }
                auto child = item->findByRouter(router.mid(1));
                if (child != nullptr) {
                    return child;
                }
            }
        }
        return nullptr;
    }
}

#define CONFIG_KEY(type, var) QJsonUtil::ConfigKey<type> var{#var}