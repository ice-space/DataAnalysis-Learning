#include <iostream>
#include <fstream>
#include <sstream>
#include <memory>
#include <string>
#include <unordered_map>
#include <openssl/sha.h>
#include <iomanip>
#include <thread>
#include <chrono>
#include <jdbc/mysql_driver.h>
#include <jdbc/mysql_connection.h>
#include <jdbc/cppconn/statement.h>
#include <jdbc/cppconn/resultset.h>
#include <jdbc/cppconn/prepared_statement.h>
#include <Windows.h>


class User {
public:
    virtual void view() = 0;
    virtual void add() = 0;
    virtual void remove() = 0;
    virtual void update() = 0;
    virtual void createView() = 0;
    virtual void logout() = 0;
};

class Admin : public User {
public:
    Admin(std::shared_ptr<sql::Connection> conn) : con_(conn) {}

    void view() override {
        // 查询系统的各种信息，例如所有用户、课程等
        std::cout << "Admin: Viewing all system information..." << std::endl;
    }

    void add() override {
        // 添加用户、课程等
        std::cout << "Admin: Adding new entries to the system..." << std::endl;
    }

    void remove() override {
        // 删除用户、课程等
        std::cout << "Admin: Removing entries from the system..." << std::endl;
    }

    void update() override {
        // 修改系统中的各种信息
        std::cout << "Admin: Updating system entries..." << std::endl;
    }

    void createView() override {
        // 创建视图
        std::cout << "Admin: Creating views..." << std::endl;
    }

    void logout() override {
        std::cout << "Admin: Logging out..." << std::endl;
        exit(0);
    }

private:
    std::shared_ptr<sql::Connection> con_;
};


class Teacher : public User {
public:
    Teacher(std::shared_ptr<sql::Connection> conn, const std::string& username)
        : con_(conn), username_(username) {}

    void view() override {
        // 查询教师自己的信息
        std::cout << "Teacher: Viewing own information..." << std::endl;
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(con_->prepareStatement(
                "SELECT * FROM Teacher WHERE Teacher_ID = (SELECT Teacher_ID FROM User WHERE Username = ?)"
            ));
            pstmt->setString(1, username_);
            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
            while (res->next()) {
                std::cout << "ID: " << res->getInt("Teacher_ID") << std::endl;
                std::cout << "Name: " << res->getString("Name") << std::endl;
                std::cout << "Gender: " << res->getString("Gender") << std::endl;
                std::cout << "Contact: " << res->getString("Contact") << std::endl;
                std::cout << "Address: " << res->getString("Address") << std::endl;
                std::cout << "Department: " << res->getString("Department") << std::endl;
            }
        }
        catch (sql::SQLException& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }
    void add() override {
        // Teachers don't have permission to add entries, provide a placeholder
        std::cout << "Teacher: No permission to add new entries." << std::endl;
    }

    void remove() override {
        // Teachers don't have permission to remove entries, provide a placeholder
        std::cout << "Teacher: No permission to remove entries." << std::endl;
    }

    void update() override {
        // Teachers don't have permission to update entries, provide a placeholder
        std::cout << "Teacher: No permission to update entries." << std::endl;
    }

    void createView() override {
        // Teachers don't have permission to create views, provide a placeholder
        std::cout << "Teacher: No permission to create views." << std::endl;
    }
    void viewStudents() {
        // 查询教师教授课程的学生名单
        std::cout << "Teacher: Viewing students list for own courses..." << std::endl;
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(con_->prepareStatement(
                "SELECT S.Student_ID, S.Name, S.Gender, S.Class_ID "
                "FROM Student S "
                "JOIN Enrollment E ON S.Student_ID = E.Student_ID "
                "JOIN Course C ON E.Course_Code = C.Course_Code "
                "WHERE C.Teacher_ID = (SELECT Teacher_ID FROM User WHERE Username = ?)"
            ));
            pstmt->setString(1, username_);
            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
            while (res->next()) {
                std::cout << "Student ID: " << res->getInt("Student_ID") << ", "
                    << "Name: " << res->getString("Name") << ", "
                    << "Gender: " << res->getString("Gender") << ", "
                    << "Class ID: " << res->getInt("Class_ID") << std::endl;
            }
        }
        catch (sql::SQLException& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    void updateGrades() {
        // 修改学生的成绩
        std::cout << "Teacher: Updating grades for students in own courses..." << std::endl;
        int studentID;
        int courseCode;
        double grade;
        std::cout << "Enter Student ID: ";
        std::cin >> studentID;
        std::cout << "Enter Course Code: ";
        std::cin >> courseCode;
        std::cout << "Enter New Grade: ";
        std::cin >> grade;

        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(con_->prepareStatement(
                "UPDATE Enrollment SET Grade = ? "
                "WHERE Student_ID = ? AND Course_Code = ? "
                "AND Course_Code IN (SELECT Course_Code FROM Course WHERE Teacher_ID = (SELECT Teacher_ID FROM User WHERE Username = ?))"
            ));
            pstmt->setDouble(1, grade);
            pstmt->setInt(2, studentID);
            pstmt->setInt(3, courseCode);
            pstmt->setString(4, username_);
            pstmt->executeUpdate();
            std::cout << "Grade updated successfully." << std::endl;
        }
        catch (sql::SQLException& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    void logout() override {
        std::cout << "Teacher: Logging out..." << std::endl;
        exit(0);
    }

private:
    std::shared_ptr<sql::Connection> con_;
    std::string username_;
};

class Student : public User {
public:
    Student(std::shared_ptr<sql::Connection> conn, const std::string& username)
        : con_(conn), username_(username) {}

    void view() override {
        // 查询学生自己的信息
        std::cout << "Student: Viewing own information..." << std::endl;
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(con_->prepareStatement(
                "SELECT * FROM Student WHERE Student_ID = (SELECT User_ID FROM User WHERE Username = ?)"
            ));
            pstmt->setString(1, username_);
            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
            while (res->next()) {
                std::cout << "ID: " << res->getInt("Student_ID") << std::endl;
                std::cout << "Name: " << res->getString("Name") << std::endl;
                std::cout << "Gender: " << res->getString("Gender") << std::endl;
                std::cout << "Class ID: " << res->getInt("Class_ID") << std::endl;
                std::cout << "Major Name: " << res->getString("Major_Name") << std::endl;
            }
        }
        catch (sql::SQLException& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    void add() override {
        // 学生角色没有增加系统信息的权限
        std::cout << "Student: No permission to add new entries." << std::endl;
    }

    void remove() override {
        // 学生角色没有删除系统信息的权限
        std::cout << "Student: No permission to remove entries." << std::endl;
    }

    void update() override {
        // 修改学生自己的信息
        std::cout << "Student: Updating own information..." << std::endl;
        // Assuming updates could be made on student's contact or address information
        // Placeholder for input fields (e.g., new address)
    }

    void createView() override {
        // 学生角色没有创建视图的权限
        std::cout << "Student: No permission to create views." << std::endl;
    }

    void viewCourses() {
        // 查询学生选课信息和成绩
        std::cout << "Student: Viewing courses and grades..." << std::endl;
        try {
            std::unique_ptr<sql::PreparedStatement> pstmt(con_->prepareStatement(
                "SELECT C.Course_Code, C.Course_Name, E.Grade "
                "FROM Course C "
                "JOIN Enrollment E ON C.Course_Code = E.Course_Code "
                "WHERE E.Student_ID = (SELECT User_ID FROM User WHERE Username = ?)"
            ));
            pstmt->setString(1, username_);
            std::unique_ptr<sql::ResultSet> res(pstmt->executeQuery());
            while (res->next()) {
                std::cout << "Course Code: " << res->getInt("Course_Code") << ", "
                    << "Course Name: " << res->getString("Course_Name") << ", "
                    << "Grade: " << res->getDouble("Grade") << std::endl;
            }
        }
        catch (sql::SQLException& e) {
            std::cerr << "Error: " << e.what() << std::endl;
        }
    }

    void logout() override {
        std::cout << "Student: Logging out..." << std::endl;
        exit(0);
    }

private:
    std::shared_ptr<sql::Connection> con_;
    std::string username_;
};


// Helper function to hash a string with SHA256
std::string sha256(const std::string& str) {
    unsigned char hash[SHA256_DIGEST_LENGTH];
    SHA256(reinterpret_cast<const unsigned char*>(str.c_str()), str.length(), hash);

    std::ostringstream oss;
    for (int i = 0; i < SHA256_DIGEST_LENGTH; ++i) {
        oss << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(hash[i]);
    }
    return oss.str();
}

class DatabaseManager {
public:
    DatabaseManager(const std::string& host, const std::string& user, const std::string& password)
        : host_(host), user_(user), password_(password), con_(nullptr) {
        SetConsoleOutputCP(CP_UTF8); // 设置编码格式，避免 MySQL 中的中文数据乱码
    }

    ~DatabaseManager() {
        disconnect();
    }

    bool connect() {
        try {
            driver_ = sql::mysql::get_mysql_driver_instance();
            con_ = std::shared_ptr<sql::Connection>(driver_->connect(host_, user_, password_));
            stmt_ = std::shared_ptr<sql::Statement>(con_->createStatement());
            con_->setAutoCommit(false);
            std::cout << "Database connected successfully." << std::endl;
            return true;
        }
        catch (sql::SQLException& e) {
            std::cout << "Connection failed: " << e.what() << std::endl;
            return false;
        }
    }

    bool checkDatabaseExists(const std::string& dbName) {
        try {
            std::unique_ptr<sql::ResultSet> res(stmt_->executeQuery("SHOW DATABASES LIKE '" + dbName + "';"));
            if (res->next()) {
                std::cout << "Database " << dbName << " exists." << std::endl;
                return true;
            }
            else {
                std::cout << "Database " << dbName << " does not exist." << std::endl;
                return false;
            }
        }
        catch (sql::SQLException& e) {
            std::cout << "Error checking database existence: " << e.what() << std::endl;
            return false;
        }
    }

    bool createDatabase(const std::string& dbName) {
        try {
            stmt_->execute("CREATE DATABASE " + dbName + ";");
            std::cout << "Database " + dbName + " created successfully." << std::endl;

            // 使用刚创建的数据库
            stmt_->execute("USE " + dbName + ";");

            // 从文件读取SQL语句
            std::string createTablesSQL;
            if (!readSQLFromFile("createdatabase.txt", createTablesSQL)) {
                std::cout << "Failed to read SQL file." << std::endl;
                return false;
            }

            // 执行读取的SQL语句
            stmt_->execute(createTablesSQL);
            std::cout << "Tables created successfully." << std::endl;

            return true;
        }
        catch (sql::SQLException& e) {
            std::cout << "Error creating database: " << e.what() << std::endl;
            return false;
        }
    }

    void disconnect() {
        if (con_) {
            try {
                con_->commit();
                std::cout << "Changes committed." << std::endl;
            }
            catch (sql::SQLException& e) {
                std::cout << "Error committing changes: " << e.what() << std::endl;
            }
            con_.reset();
            stmt_.reset();
            std::cout << "Database connection closed." << std::endl;
        }
    }

    std::shared_ptr<sql::Connection> getConnection() {
        return con_;
    }
    bool login(const std::string& username, const std::string& password) {
        try {
            std::unique_ptr<sql::ResultSet> res(stmt_->executeQuery("SELECT Role, Password FROM User WHERE Username = '" + username + "';"));
            if (res->next()) {
                std::string dbPassword = res->getString("Password");
                std::string role = res->getString("Role");

                if (sha256(password) == dbPassword) {
                    std::cout << "Login successful. Welcome, " << username << "!" << std::endl;
                    if (role == "admin") {
                        // Admin role
                        Admin admin(con_);
                        adminInterface(admin);
                    }
                    else if (role == "teacher") {
                        // Teacher role
                        Teacher teacher(con_, username);
                        teacherInterface(teacher);
                    }
                    else if (role == "student") {
                        // Student role
                        Student student(con_, username);
                        studentInterface(student);
                    }
                    return true;
                }
                else {
                    std::cout << "Invalid username or password." << std::endl;
                    return false;
                }
            }
            else {
                std::cout << "User not found." << std::endl;
                return false;
            }
        }
        catch (sql::SQLException& e) {
            std::cout << "Error during login: " << e.what() << std::endl;
            return false;
        }
    }

    void adminInterface(Admin& admin) {
        // 展示管理员操作菜单
        std::cout << "Admin Menu:" << std::endl;
        std::cout << "1. View information" << std::endl;
        std::cout << "2. Add entries" << std::endl;
        std::cout << "3. Remove entries" << std::endl;
        std::cout << "4. Update entries" << std::endl;
        std::cout << "5. Create view" << std::endl;
        std::cout << "0. Log Out" << std::endl;
        std::cout << "Please select an option: ";
        int option;
        std::cin >> option;
        switch (option) {
        case 1: admin.view(); break;
        case 2: admin.add(); break;
        case 3: admin.remove(); break;
        case 4: admin.update(); break;
        case 5: admin.createView(); break;
        case 0: admin.logout(); break;
        default: std::cout << "Invalid option." << std::endl; break;
        }
    }

    void teacherInterface(Teacher& teacher) {
        // 展示教师操作菜单
        std::cout << "Teacher Menu:" << std::endl;
        std::cout << "1. View own information" << std::endl;
        std::cout << "2. View students list" << std::endl;
        std::cout << "3. Update grades" << std::endl;
        std::cout << "0. Log Out" << std::endl;
        std::cout << "Please select an option: ";
        int option;
        std::cin >> option;
        switch (option) {
        case 1: teacher.view(); break;
        case 2: teacher.viewStudents(); break;
        case 3: teacher.updateGrades(); break;
        case 0: teacher.logout(); break;
        default: std::cout << "Invalid option." << std::endl; break;
        }
    }

    void studentInterface(Student& student) {
        // 展示学生操作菜单
        std::cout << "Student Menu:" << std::endl;
        std::cout << "1. View own information" << std::endl;
        std::cout << "2. View courses and grades" << std::endl;
        std::cout << "0. Log Out" << std::endl;
        std::cout << "Please select an option: ";
        int option;
        std::cin >> option;
        switch (option) {
        case 1: student.view(); break;
        case 2: student.viewCourses(); break;
        case 0: student.logout(); break;
        default: std::cout << "Invalid option." << std::endl; break;
        }
    }

private:
    std::string host_;
    std::string user_;
    std::string password_;
    sql::mysql::MySQL_Driver* driver_;
    std::shared_ptr<sql::Connection> con_;
    std::shared_ptr<sql::Statement> stmt_;

    bool readSQLFromFile(const std::string& filename, std::string& sql) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            return false;
        }

        std::stringstream buffer;
        buffer << file.rdbuf();
        sql = buffer.str();

        file.close();
        return true;
    }

    void adminInterface() {
        // Implement the admin interface here
    }

    void teacherInterface() {
        // Implement the teacher interface here
    }

    void studentInterface() {
        // Implement the student interface here
    }
};







int main() {
    std::string host = "xxx.xxx.xxx.xxx";
    std::string user = "root";
    std::string password = "123456789";

    DatabaseManager dbManager(host, user, password);

    if (!dbManager.connect()) {
        std::cout << "Connection failed. Please try again." << std::endl;
        return 1;
    }

    std::string dbName = "FinalWork";
    if (!dbManager.checkDatabaseExists(dbName)) {
        std::cout << "Do you want to create the database? (yes/no): ";
        std::string response;
        std::cin >> response;
        if (response == "yes" || response == "Yes" || response == "YES") {
            if (!dbManager.createDatabase(dbName)) {
                std::cout << "Failed to create database. Exiting..." << std::endl;
                return 1;
            }
        }
        else {
            std::cout << "Thank you for using the program. Goodbye!" << std::endl;
            return 0;
        }
    }

    // 登录尝试
    int loginAttempts = 0;
    const int maxAttempts = 5;
    bool loggedIn = false;

    while (loginAttempts < maxAttempts && !loggedIn) {
        std::string username;
        std::string password;

        std::cout << "Enter username: ";
        std::cin >> username;
        std::cout << "Enter password: ";
        std::cin >> password;

        loggedIn = dbManager.login(username, password);

        if (!loggedIn) {
            loginAttempts++;
            if (loginAttempts < maxAttempts) {
                std::cout << "You have " << maxAttempts - loginAttempts << " attempt(s) left." << std::endl;
            }
        }
    }

    if (!loggedIn) {
        std::cout << "Too many failed attempts. Exiting in 10 seconds..." << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(10));
        return 1;
    }

    // 执行完操作后，询问是否保存更改
    std::cout << "Do you want to save changes before exiting? (yes/no): ";
    std::string saveResponse;
    std::cin >> saveResponse;
    if (saveResponse == "yes" || saveResponse == "Yes" || saveResponse == "YES") {
        dbManager.disconnect(); // 提交更改并断开连接
    }
    else {
        try {
            dbManager.getConnection()->rollback(); // 回滚未提交的更改
            std::cout << "Changes not saved. Rolling back..." << std::endl;
        }
        catch (sql::SQLException& e) {
            std::cout << "Error rolling back changes: " << e.what() << std::endl;
        }
        dbManager.disconnect();
    }

    return 0;
}
