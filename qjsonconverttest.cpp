#include <qobject.h>
#include <QTest>

#include "classes.h"

class QJsonConvertTest : public QObject {
    Q_OBJECT

private slots:
    void convertTest() {
        const QByteArray jsonStr = R"(
{
    "name": "class1",
    "room": 1,
    "courses": ["math", "english", "physics", "chemistry", "biology"],
    "types": [0, 1, 2],
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

            auto json = classes.dumpToJson();
            QCOMPARE(QJsonDocument(json).toJson(), doc.toJson());

            //read
            auto aliceName = classes.students().first().name();
            QCOMPARE(aliceName, "Alice");

            auto types = classes.types();
            QList<int> expectTypes = {0, 1, 2};
            QCOMPARE(types, expectTypes);

            //write
            classes.teacher().score = 98.1;

            //read by string
            DataKey<int>* room = classes.findByRouter<int>("room");
            QCOMPARE((*room)(), 1);

            DataKey<QString>* teacherName = classes.findByRouter<QString>("teacher.name");
            QCOMPARE((*teacherName)(), "Tony");

            DataKey<QList<Student>>* students = classes.findByRouter<QList<Student>>("students");
            QCOMPARE((*students)().size(), 2);

            DataKey<int>* aliceAge = classes.findByRouter<int>("students.0.age");
            QCOMPARE((*aliceAge)(), 12);

            DataKey<QStringList>* aliceAdept = classes.findByRouter<QStringList>("students.0.adept");
            QCOMPARE((*aliceAdept)(), QStringList() << "math" << "english");

            DataKey<QString>* nestedValue = classes.findByRouter<QString>("nestedValues.0.0.name");
            QCOMPARE((*nestedValue)(), "math");
        }
    }
};

#include "qjsonconverttest.moc"

QTEST_MAIN(QJsonConvertTest)