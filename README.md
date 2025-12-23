University DBMS (Qt + MySQL)

A simple University Database Management System built using Qt (C++) and MySQL, supporting student, course, and enrollment management through a graphical user interface.

Features
	•	User login (Admin, Lecturer, Student, Staff)
	•	Manage Students (Add, Update, Delete)
	•	Manage Courses (Add, Update, Delete)
	•	Manage Enrollment (Assign courses to students, update grades)
	•	MySQL database connection using QSqlDatabase
	•	Real-time table updates using QSqlTableModel

Project Files
main.cpp
mainwindow.cpp
mainwindow.h
mainwindow.ui
UniversityDatabaseManagementSystemApp.pro

Database Connection (Qt Code)
db = QSqlDatabase::addDatabase("QMYSQL");
db.setHostName("localhost");
db.setPort(3306);
db.setDatabaseName("UniversityDB");
db.setUserName("root");
db.setPassword("YOUR_PASSWORD");

db.open();

How to Run
	1.	Open .pro file in Qt Creator
	2.	Configure MySQL database
	3.	Build & Run the project

Login Example
Username: admin1
Password: admin123
Role: Admin

License

MIT License.
