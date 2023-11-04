- datakey.h  
c++结构体与json/xml字符串相互转换工具类

--- 

### 1. json字符串转换  

json字符串：
```json
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
```

定义模型：

```cpp
#include "include/configkey.h"

using namespace QDataUtil;

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

    DATA_KEY(int, room);

    DATA_KEY(QStringList, courses);

    DATA_KEY(Teacher, teacher);

    DATA_KEY(QList<Student>, students);
    
    DATA_KEY(QList<int>, types);

    DATA_KEY(QList<QList<CourseInfo>>, nestedValues);

    QList<DataReadInterface *> prop() override {
        return {&name, &room, &courses, &teacher, &students, &types, &nestedValues};
    }
};
```

转换：

```cpp
auto doc = QJsonDocument::fromJson(jsonStr);
if (!doc.isNull()) {
  auto object = doc.object();

  Classes classes;
  classes.fromJson(object);

  auto json = classes.dumpToJson();
}
```

字符串查找

```cpp
auto room = classes.findByRouter<int, TestProperty>("room");

auto teacherName = classes.findByRouter<QString>("teacher.name");

auto students = classes.findByRouter<QList<Student>>("students");

auto aliceAge = classes.findByRouter<int>("students.0.age");

auto aliceAdept = classes.findByRouter<QStringList>("students.0.adept");

auto nestedValue = classes.findByRouter<QString>("nestedValues.0.0.name");
```

### 2. xml字符串转换  

xml字符串：

```xml
<?xml version="1.0" encoding="UTF-8"?>
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
</class>
```

定义模型：

```cpp
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

    //字段属性定义
    DATA_KEY(int, room, TestProperty);

    DATA_KEY(QStringList, courses);

    DATA_KEY(Teacher, teacher);

    //列表属性定义
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
```

转换：

```cpp
Classes classes;
classes.fromXml(reader);

QFile file("xml_buff.xml");
file.open(QIODevice::WriteOnly | QIODevice::Truncate);
QXmlStreamWriter writer(&file);
classes.dumpToXml(writer, true);
file.close();
```

字符串查找  

```cpp
auto room = classes.findByRouter<int, TestProperty>("room");

auto teacherName = classes.findByRouter<QString>("teacher.name");

auto students = classes.findByRouter<QList<Student>, QList<TestProperty>>("students");

auto aliceAge = classes.findByRouter<int>("students.0.age");

auto aliceAdept = classes.findByRouter<QStringList>("students.0.adept");
```