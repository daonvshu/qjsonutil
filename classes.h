#include "include/datakey.h"

using namespace QDataUtil;

struct TestProperty : DataDumpInterface {

    DATA_KEY(QString, lang);

    DATA_KEY(QString, type);

    DATA_KEY(int, value);

    QList<DataReadInterface *> prop() override {
        return { &lang, &type, &value };
    }
};

struct Student : DataDumpInterface {

    DATA_KEY(QString, name);

    DATA_KEY(int, age);

    //字段与key名称不一致时不使用宏
    DataKey<double> scoreAvg{"score_avg"};

    DATA_KEY(QStringList, adept);

    QList<DataReadInterface *> prop() override {
        return {&name, &age, &scoreAvg, &adept};
    }
};

struct Teacher : DataDumpInterface {

    DATA_KEY(QString, name);

    DATA_KEY(double, score);

    QList<DataReadInterface *> prop() override {
        return {&name, &score};
    }
};

struct CourseInfo : DataDumpInterface {

    DATA_KEY(QString, name);

    DATA_KEY(int, index);

    QList<DataReadInterface *> prop() override {
        return { &name, &index };
    }
};

struct Classes : DataDumpInterface {

    DATA_KEY(QString, name);

    DATA_KEY(int, room, TestProperty);

    DATA_KEY(QStringList, courses);

    DATA_KEY(Teacher, teacher);

    DATA_KEY(QList<Student>, students, QList<TestProperty>);

    DATA_KEY(QList<int>, types);

    DATA_KEY(QList<QList<CourseInfo>>, nestedValues);

    QList<DataReadInterface *> prop() override {
        return {&name, &room, &courses, &teacher, &students, &types, &nestedValues };
    }

    QString groupKey() override {
        return "class";
    }
};