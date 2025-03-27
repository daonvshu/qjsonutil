#include <qobject.h>
#include <QTest>

#include "classes.h"

class DataConvertTest : public QObject {
    Q_OBJECT

private slots:
    static void jsonConvertTest() {
        const QByteArray jsonStr = R"(
{
    "name": "class1",
    "name2": "class2",
    "room": 1,
    "courses": ["math", "english", "physics", "chemistry", "biology"],
    "types": [0, 1, 2],
    "customObj": {
        "name": "customObjValue",
        "value": 10
    },
    "customObj2": {
        "name": "customObj2Value",
        "value": 20
    },
    "customObj3": [
        {
            "name": "customObj3Value",
            "value": 30
        }
    ],
    "nestedValues": [
        [
            {
                "name": "math",
                "index": 0
            },
            {
                "name": "english",
                "index": 1
            }
        ]
    ],
    "teacher": {
        "name": "Tony",
        "score": 99.9
    },
    "students": [
        {
            "name": "Alice",
            "age": 12,
            "score_avg": 90.4,
            "adept": ["math", "english"]
        },
        {
            "name": "Bob",
            "age": 13,
            "score_avg": 86.1,
            "adept": ["physics", "chemistry"]
        }
    ]
}
        )";

        auto doc = QJsonDocument::fromJson(jsonStr);
        if (!doc.isNull()) {
            auto object = doc.object();

            Classes classes;
            classes.fromJson(object);

            constDumpFunction(classes);

            //classes.name = classes.name2;

            auto json = classes.dumpToJson();
            auto dumpStr = QJsonDocument(json).toJson();
            QCOMPARE(dumpStr, doc.toJson());

            auto customObjStr = QJsonDocument(classes.customObj().toObject()).toJson();
            QVERIFY(!customObjStr.isEmpty());
            QCOMPARE(classes.customObj(), object.value("customObj").toObject());

            customObjStr = QJsonDocument(classes.customObj2()).toJson();
            QVERIFY(!customObjStr.isEmpty());
            QCOMPARE(classes.customObj2(), object.value("customObj2").toObject());

            customObjStr = QJsonDocument(classes.customObj3()).toJson();
            QVERIFY(!customObjStr.isEmpty());
            QCOMPARE(classes.customObj3(), object.value("customObj3").toArray());

            //read
            auto aliceName = classes.students().first().name();
            QCOMPARE(aliceName, "Alice");

            auto types = classes.types();
            QList<int> expectTypes = {0, 1, 2};
            QCOMPARE(types, expectTypes);

            //write
            classes.teacher().score = 98.1;

            //read by string
            DataKey<int, TestProperty>* room = classes.findByRouter<int, TestProperty>("room");
            QCOMPARE((*room)(), 1);

            DataKey<QString>* teacherName = classes.findByRouter<QString>("teacher.name");
            QCOMPARE((*teacherName)(), "Tony");

            DataKey<QList<Student>, QList<TestProperty>>* students = classes.findByRouter<QList<Student>, QList<TestProperty>>("students");
            QCOMPARE((*students)().size(), 2);

            DataKey<int>* aliceAge = classes.findByRouter<int>("students.0.age");
            QCOMPARE((*aliceAge)(), 12);

            DataKey<QStringList>* aliceAdept = classes.findByRouter<QStringList>("students.0.adept");
            QCOMPARE((*aliceAdept)(), QStringList() << "math" << "english");

            DataKey<QString>* nestedValue = classes.findByRouter<QString>("nestedValues.0.0.name");
            QCOMPARE((*nestedValue)(), "math");
        }
    }

    static void xmlConvertTest() {
        const QByteArray xmlStr = R"(<?xml version="1.0" encoding="UTF-8"?>
<class>
    <name>class1</name>
    <room lang="en" type="room" value="1">1</room>
    <courses>math</courses>
    <courses>english</courses>
    <courses>physics</courses>
    <courses>chemistry</courses>
    <courses>biology</courses>
    <types>0</types>
    <types>1</types>
    <types>2</types>
    <teacher>
        <name>Tony</name>
        <score>99.9</score>
    </teacher>
    <students lang="en" type="student1" value="2">
        <name>Alice</name>
        <age>12</age>
        <score_avg>90.4</score_avg>
        <adept>math</adept>
        <adept>english</adept>
    </students>
    <students lang="en" type="student2" value="3">
        <name>Bob</name>
        <age>13</age>
        <score_avg>86.1</score_avg>
        <adept>physics</adept>
        <adept>chemistry</adept>
    </students>
</class>)";

        QXmlStreamReader reader(xmlStr);
        {
            Classes classes;
            classes.fromXml(reader);

            QFile file("xml_buff.xml");
            file.open(QIODevice::WriteOnly | QIODevice::Truncate);
            QXmlStreamWriter writer(&file);
            classes.dumpToXml(writer, true);
            file.close();

            //read
            auto aliceName = classes.students().first().name();
            QCOMPARE(aliceName, "Alice");

            auto types = classes.types();
            QList<int> expectTypes = {0, 1, 2};
            QCOMPARE(types, expectTypes);

            //write
            classes.teacher().score = 98.1;

            //read by string
            DataKey<int, TestProperty>* room = classes.findByRouter<int, TestProperty>("room");
            QCOMPARE((*room)(), 1);
            QCOMPARE(room->dataProperty.type(), "room");

            DataKey<QString>* teacherName = classes.findByRouter<QString>("teacher.name");
            QCOMPARE((*teacherName)(), "Tony");

            DataKey<QList<Student>, QList<TestProperty>>* students = classes.findByRouter<QList<Student>, QList<TestProperty>>("students");
            QCOMPARE((*students)().size(), 2);

            DataKey<int>* aliceAge = classes.findByRouter<int>("students.0.age");
            QCOMPARE((*aliceAge)(), 12);

            DataKey<QStringList>* aliceAdept = classes.findByRouter<QStringList>("students.0.adept");
            QCOMPARE((*aliceAdept)(), QStringList() << "math" << "english");

            //DataKey<QString>* nestedValue = classes.findByRouter<QString>("nestedValues.0.0.name");
            //QCOMPARE((*nestedValue)(), "math");
        }
    }

    static QJsonObject constDumpFunction(const Classes& classes) {
        return classes.dumpToJson();
    }
};

#include "dataconverttest.moc"

QTEST_MAIN(DataConvertTest)