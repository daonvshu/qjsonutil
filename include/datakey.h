#pragma once

#include <qjsonobject.h>
#include <qjsonarray.h>
#include <qjsondocument.h>
#include <qxmlstream.h>
#include <qvariant.h>
#include <qdebug.h>

namespace QDataUtil {

    struct DataReadInterface {
        //读取json的key
        virtual const QString& key() const = 0;
        //value转成QJsonValue
        virtual QJsonValue value() = 0;
        //写入json值
        virtual void save(const QJsonValue& value, bool keyVerify) = 0;
        //写入xml值
        virtual void save(QXmlStreamReader& value, bool keyVerify) = 0;
        //存储xml值
        virtual void restore(QXmlStreamWriter& writer) = 0;
        //存储xml属性
        virtual void restoreProperty(QXmlStreamWriter& writer) = 0;

        //字符串路由
        virtual DataReadInterface* findByRouter(const QStringList& router) = 0;

        virtual ~DataReadInterface() = default;
    };

    template<typename T, typename Property>
    struct DataKey;

    struct DataNonProperty {};

    struct DataDumpInterface {

        //将JsonObject转换为模型字段
        virtual void fromJson(const QJsonObject &jsonObject, bool keyVerify = true) {
            auto objKeys = jsonObject.keys();
            for (const auto& item : prop()) {
                if (keyVerify) {
#ifdef QT_DEBUG
                    // not found the key in source data
                    Q_ASSERT(jsonObject.contains(item->key())); //check all props in object keys
                    objKeys.removeOne(item->key());
#else
                    if (!jsonObject.contains(item->key())) {
                        qWarning() << "[QDataUtil] not found the key" << item->key() << "in source data!";
                        continue;
                    }
                    objKeys.removeOne(item->key());
#endif
                } else {
                    if (!jsonObject.contains(item->key())) {
                        continue;
                    }
                }
                item->save(jsonObject.value(item->key()), keyVerify);
            }
            if (keyVerify) {
#ifdef QT_DEBUG
                // forget add all fields in function 'prop()' ?
                Q_ASSERT(objKeys.isEmpty()); //check all object keys used in props
#else
                if (!objKeys.isEmpty()) {
                    qWarning() << "[QDataUtil] forget add field in function 'prop()' ? the fields:" << objKeys;
                }
#endif
            }
        }

        //将模型结构体转JsonObject
        virtual QJsonObject dumpToJson() const {
            QJsonObject jsonObject;
            for (const auto& item : constProp()) {
                jsonObject.insert(item->key(), item->value());
            }
            return jsonObject;
        }

        //将QXmlStreamReader转换为模型字段
        virtual void fromXmlReader(QXmlStreamReader &reader, bool keyVerify = true) {
            QHash<QString, DataReadInterface*> infMap;
            for (auto& item : prop()) {
                infMap.insert(item->key(), item);
            }
            reader.readNext();
            int depth = 0;
            while (!reader.atEnd()) {
                if (reader.isStartElement()) {
                    depth++;
                    auto key = reader.name().toString();
                    if (keyVerify) {
#ifdef QT_DEBUG
                        // forget add all fields in function 'prop()' ?
                        Q_ASSERT(infMap.contains(key)); //check all object keys in props
                        infMap.take(key)->save(reader, keyVerify);
#else
                        if (infMap.contains(key)) {
                            infMap[key]->save(reader, keyVerify);
                        } else {
                            qWarning() << "[QDataUtil] forget add field in function 'prop()' ? the field:" << key;
                        }
#endif
                    } else {
                        if (infMap.contains(key)) {
                            infMap[key]->save(reader, keyVerify);
                        }
                    }
                    reader.readNext();
                } else if (reader.isEndElement()) {
                    depth--;
                    if (depth == 0) {
                        break;
                    }
                } else {
                    reader.readNext();
                }
            }
            if (keyVerify) {
#ifdef QT_DEBUG
                // not found keys in source data
                Q_ASSERT(infMap.isEmpty()); //check all props used in object keys
#else
                if (!infMap.isEmpty()) {
                    qWarning() << "[QDataUtil] not found keys in source data! the keys:" << infMap.keys();
                }
#endif
            }
        }

        //将QXmlStreamReader转换为模型字段
        virtual void fromXml(QXmlStreamReader &reader, bool keyVerify = true) {
            reader.readNext();
            while (!reader.atEnd()) {
                if (reader.isStartElement()) {
                    auto key = reader.name().toString().toLower();
                    if (key == groupKey().toLower()) {
                        fromXmlReader(reader, keyVerify);
                    } else { //is not group begin
                        break;
                    }
                } else {
                    reader.readNext();
                }
            }
        }

        //将模型结构体转QXmlStreamWriter
        virtual void dumpToXml(QXmlStreamWriter& writer, const QString& elementName,
                               const std::function<void(QXmlStreamWriter&)>& propertyWriter = nullptr) const {
            writer.writeStartElement(elementName);
            if (propertyWriter) {
                propertyWriter(writer);
            }
            for (auto& item : constProp()) {
                item->restore(writer);
            }
            writer.writeEndElement();
        }

        //将模型结构体转QXmlStreamWriter
        virtual void dumpToXml(QXmlStreamWriter& writer, bool autoFormat) const {
            writer.setAutoFormatting(autoFormat);
            writer.writeStartDocument();
            dumpToXml(writer, constGroupKey());
            writer.writeEndDocument();
        }

        //读取模型类所有字段
        virtual QList<DataReadInterface*> prop() = 0;

        QList<DataReadInterface*> constProp() const {
            return const_cast<DataDumpInterface*>(this)->prop();
        }

        //顶层xml标签
        virtual QString groupKey() {
            return {};
        }

        QString constGroupKey() const {
            return const_cast<DataDumpInterface*>(this)->groupKey();
        }

        //通过字符串路由查找模型字段
        template<typename T, typename Property = DataNonProperty>
        DataKey<T, Property>* findByRouter(const QString& router) const;

        DataReadInterface* findByRouter(const QStringList& router) const;

        virtual ~DataDumpInterface() = default;
    };

    template<typename T>
    struct DataIdentity {
        using type = T;
    };

    template<typename I> struct DataIteratorType;
    template<typename I>
    struct DataIteratorType<QList<I>> {
        using type = I;
    };

    template<typename T, typename Property = DataNonProperty>
    struct DataKey : DataReadInterface {
        //保存data对应的key名称
        QString dataKey;
        //保存存储的值
        T dataValue;
        //xml对应的属性结构体
        Property dataProperty;

        //初始化时将key传入
        explicit DataKey(QString key)
                : dataKey(std::move(key)), dataValue(T())
        {}

        //禁用对象拷贝
        DataKey& operator=(DataKey<T>&) = delete;

        //赋值
        DataKey& operator=(const T& v) {
            dataValue = v;
            return *this;
        }

        //引用取值
        T& operator()() {
            return dataValue;
        }

        //const取值
        const T& operator()() const {
            return dataValue;
        }

        //读取key
        const QString& key() const override {
            return dataKey;
        }

        template<typename K>
        using ValueType = typename std::conditional<std::is_base_of<DataDumpInterface, K>::value, DataDumpInterface, K>::type;

        //value转QJsonValue
        QJsonValue value() override {
            return toJsonValue(dataValue, DataIdentity<ValueType<T>>());
        }
        
        //保存
        void save(const QJsonValue &value, bool keyVerify) override {
            fromJsonValue(dataValue, value, DataIdentity<ValueType<T>>(), keyVerify);
        }

        //存储
        void restore(QXmlStreamWriter &writer) override {
            toXmlValue(dataKey, dataValue, dataProperty, writer, DataIdentity<ValueType<T>>());
        }

        //存储属性
        void restoreProperty(QXmlStreamWriter &writer) override {
            writeXmlProperty<T>(writer, dataValue);
        }

        //保存
        void save(QXmlStreamReader &value, bool keyVerify) override {
            fromXmlProperty(dataProperty, value, DataIdentity<ValueType<Property>>(), keyVerify);
            fromXmlValue(dataValue, value, DataIdentity<ValueType<T>>(), keyVerify);
        }

        //通过字符串路由判断是否是自己
        DataReadInterface* findByRouter(const QStringList& router) override {
            return findByRouter(router, DataIdentity<ValueType<T>>());
        }

    private:
        //基础类型转换
        template<typename I, typename K>
        static QJsonValue toJsonValue(I& value, DataIdentity<K>) {
            return value;
        }

        //DataKey类型
        template<typename I>
        static QJsonValue toJsonValue(I& value, DataIdentity<DataDumpInterface>) {
            return dynamic_cast<DataDumpInterface*>(&value)->dumpToJson();
        }

        //容器类型
        template<typename I, typename K>
        static QJsonValue toJsonValue(I& value, DataIdentity<QList<K>>) {
            QJsonArray jsonArray;
            for (auto& v : value) {
                jsonArray.append(toJsonValue(v, DataIdentity<ValueType<K>>()));
            }
            return jsonArray;
        }

        //QStringList类型
        template<typename I>
        static QJsonValue toJsonValue(I& value, DataIdentity<QStringList>) {
            return QJsonArray::fromStringList(value);
        }

        //基础类型转换
        template<typename I, typename P, typename K>
        static void toXmlValue(const QString& key, I& value, P& prop, QXmlStreamWriter &writer, DataIdentity<K>) {
            writer.writeStartElement(key);
            toXmlProperty(prop, writer, DataIdentity<ValueType<P>>());
            writer.writeCharacters(QVariant(value).toString());
            writer.writeEndElement();
        }

        //DataKey类型
        template<typename I, typename P>
        static void toXmlValue(const QString& key, I& value, P& prop, QXmlStreamWriter &writer, DataIdentity<DataDumpInterface>) {
            dynamic_cast<DataDumpInterface*>(&value)->dumpToXml(writer, key, [&] (QXmlStreamWriter& w) {
                toXmlProperty(prop, w, DataIdentity<ValueType<P>>());
            });
        }

        //容器类型
        template<typename I, typename P, typename K>
        static void toXmlValue(const QString& key, I& value, P& prop, QXmlStreamWriter &writer, DataIdentity<QList<K>>) {
            for (int i = 0; i < value.size(); i++) {
                toXmlValue(key, value[i], getXmlPropertyFromList(prop, i, DataIdentity<ValueType<P>>()), writer, DataIdentity<ValueType<K>>());
            }
        }

        //QStringList类型
        template<typename I, typename P>
        static void toXmlValue(const QString& key, I& value, P& prop, QXmlStreamWriter &writer, DataIdentity<QStringList>) {
            for (int i = 0; i < value.size(); i++) {
                writer.writeStartElement(key);
                toXmlProperty(getXmlPropertyFromList(prop, i, DataIdentity<ValueType<P>>()), writer, DataIdentity<P>());
                writer.writeCharacters(QVariant(value[i]).toString());
                writer.writeEndElement();
            }
        }

        template<typename P>
        static P& getXmlPropertyFromList(P& prop, int, DataIdentity<P>) {
            return prop;
        }

        template<typename K>
        static K& getXmlPropertyFromList(QList<K>& prop, int index, DataIdentity<QList<K>>) {
            return prop[index];
        }

        //基础类型转换
        template<typename I, typename K>
        static void toXmlProperty(I&, QXmlStreamWriter&, DataIdentity<K>) {
        }

        //DataKey类型
        template<typename I>
        static void toXmlProperty(I& value, QXmlStreamWriter &writer, DataIdentity<DataDumpInterface>) {
            for (auto& item : value.prop()) {
                item->restoreProperty(writer);
            }
        }

        template<typename K>
        void writeXmlProperty(QXmlStreamWriter& writer, const typename std::enable_if<std::is_constructible<QVariant, K>::value, K>::type& v) {
            writer.writeAttribute(dataKey, QVariant(v).toString());
        }

        template<typename K>
        void writeXmlProperty(QXmlStreamWriter&, const typename std::enable_if<!std::is_constructible<QVariant, K>::value, K>::type& v) {
        }

        //基础类型赋值
        template<typename I, typename K>
        static void fromJsonValue(I& tagValue, const QJsonValue& value, DataIdentity<K>, bool) {
            tagValue = value.toVariant().value<K>();
        }

        //DataKey类型
        template<typename I>
        static void fromJsonValue(I& tagValue, const QJsonValue& value, DataIdentity<DataDumpInterface>, bool keyVerify) {
            dynamic_cast<DataDumpInterface*>(&tagValue)->fromJson(value.toObject(), keyVerify);
        }

        //容器类型
        template<typename I, typename K>
        static void fromJsonValue(I& tagValue, const QJsonValue& value, DataIdentity<QList<K>>, bool keyVerify) {
            tagValue = QList<K>();
            auto values = value.toArray();
            for (const auto& v : values) {
                typename DataIteratorType<QList<K>>::type temp;
                fromJsonValue(temp, v, DataIdentity<ValueType<K>>(), keyVerify);
                tagValue.append(temp);
            }
        }

        //基础类型赋值
        template<typename I, typename K>
        static void fromXmlValue(I& tagValue, QXmlStreamReader &value, DataIdentity<K>, bool) {
            tagValue = QVariant(value.readElementText()).value<K>();
        }

        //DataKey类型
        template<typename I>
        static void fromXmlValue(I& tagValue, QXmlStreamReader &value, DataIdentity<DataDumpInterface>, bool keyVerify) {
            dynamic_cast<DataDumpInterface*>(&tagValue)->fromXmlReader(value, keyVerify);
        }

        //容器类型
        template<typename I, typename K>
        static void fromXmlValue(I& tagValue, QXmlStreamReader &value, DataIdentity<QList<K>>, bool keyVerify) {
            while (!value.atEnd()) {
                if (value.isStartElement()) {
                    typename DataIteratorType<QList<K>>::type temp;
                    fromXmlValue(temp, value, DataIdentity<ValueType<K>>(), keyVerify);
                    tagValue.append(temp);
                } else if (value.isEndElement()) {
                    break;
                } else {
                    value.readNext();
                }
            }
        }

        template<typename I>
        static void fromXmlValue(I& tagValue, QXmlStreamReader &value, DataIdentity<QStringList>, bool keyVerify) {
            fromXmlValue(tagValue, value, DataIdentity<QList<QString>>(), keyVerify);
        }

        template<typename I, typename K>
        static void fromXmlProperty(I&, QXmlStreamReader&, DataIdentity<K>, bool) {
        }

        template<typename I>
        static void fromXmlProperty(I& value, QXmlStreamReader &reader, DataIdentity<DataDumpInterface>, bool keyVerify) {
            QHash<QString, DataReadInterface*> infMap;
            for (auto& item : value.prop()) {
                infMap.insert(item->key(), item);
            }
            for (const auto& attribute : reader.attributes()) {
                QString key = attribute.name().toString();
                if (infMap.contains(key)) {
                    infMap[key]->save(QJsonValue::fromVariant(QVariant(attribute.value().toString())), keyVerify);
                }
            }
        }

        template<typename I, typename K>
        static void fromXmlProperty(I& value, QXmlStreamReader& reader, DataIdentity<QList<K>>, bool keyVerify) {
            typename DataIteratorType<QList<K>>::type temp;
            fromXmlProperty(temp, reader, DataIdentity<ValueType<K>>(), keyVerify);
            value.append(temp);
        }

        template<typename K>
        DataReadInterface* findByRouter(const QStringList& router, DataIdentity<K>) {
            if (router.length() == 1 && router.first() == dataKey) {
                return this;
            }
            return nullptr;
        }

        DataReadInterface* findByRouter(const QStringList& router, DataIdentity<DataDumpInterface>) {
            return dynamic_cast<DataDumpInterface*>(&dataValue)->findByRouter(router);
        }

        template<typename K>
        DataReadInterface* findByRouter(const QStringList& router, DataIdentity<QList<K>>) {
            return findNextRouter(dataValue, router, DataIdentity<QList<K>>());
        }

        template<typename K>
        static DataReadInterface* readerCaster(typename std::enable_if<std::is_base_of<DataReadInterface, K>::value, K*>::type value) {
            return dynamic_cast<DataReadInterface*>(value);
        }

        template<typename K>
        static DataReadInterface* readerCaster(typename std::enable_if<!std::is_base_of<DataReadInterface, K>::value, K*>::type) {
            return nullptr;
        }

        template<typename K>
        static DataReadInterface* findNextRouter(typename std::enable_if<std::is_base_of<DataDumpInterface, K>::value, K>::type& value, const QStringList& router) {
            return value.findByRouter(router);
        }

        template<typename K>
        static DataReadInterface* findNextRouter(typename std::enable_if<!std::is_base_of<DataDumpInterface, K>::value, K>::type& value, const QStringList& router) {
            return findNextRouter(value, router, DataIdentity<K>());
        }

        template<typename K>
        static DataReadInterface* findNextRouter(K&, const QStringList&, DataIdentity<K>) {
            return nullptr;
        }

        template<typename K>
        static DataReadInterface* findNextRouter(QList<K>& value, const QStringList& router, DataIdentity<QList<K>>) {
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

    template<typename T, typename Property>
    inline DataKey<T, Property> *DataDumpInterface::findByRouter(const QString &router) const {
        auto routerList = router.split(".");
        if (routerList.isEmpty()) {
            return nullptr;
        }
        return dynamic_cast<DataKey<T, Property>*>(findByRouter(routerList));
    }

    inline DataReadInterface *DataDumpInterface::findByRouter(const QStringList &router) const {
        auto& key = router.first();
        for (auto item : constProp()) {
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

#define DATA_KEY(type, var, ...) QDataUtil::DataKey<type, ##__VA_ARGS__> var{#var}
