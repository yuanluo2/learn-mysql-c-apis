#include <iostream>
#include <format>
#include <string>
#include <boost/locale.hpp>
#include <mysql/mysql.h>

using namespace std::literals::string_literals;

// for windows platform, because my default chcp is 936.
inline std::string utf8ToGbk(std::string_view str) {
	return boost::locale::conv::between(str.data(), "GBK", "UTF-8");
}

inline std::string gbkToUtf8(std::string_view str) {
	return boost::locale::conv::between(str.data(), "UTF-8", "GBK");
}

void test_query(MYSQL* mysql) {
	std::string query = "SELECT * FROM m_blog"s;
	if (0 != mysql_real_query(mysql, query.c_str(), query.size())) {
		std::cout << std::format("mysql_real_query error: {}.\n", mysql_error(mysql));
		return;
	}

	MYSQL_RES* result = mysql_store_result(mysql);
	if (nullptr == result) {
		std::cout << std::format("mysql_store_result error: {}.\n", mysql_error(mysql));
		return;
	}

	auto numOfRows = mysql_num_rows(result);
	std::cout << std::format("result contains {} rows.\n", numOfRows);

	auto numOfFields = mysql_num_fields(result);

	while (auto row = mysql_fetch_row(result)) {
		for (auto i = 0; i < numOfFields;++i) {
			auto* field = mysql_fetch_field_direct(result, i);

			if ("title"s == field->name) {
				std::cout << std::format("{}\n", utf8ToGbk(row[i]));     
			}
		}
	}

	mysql_free_result(result);
}

void test_insert(MYSQL* mysql) {
	std::string query = std::format("INSERT INTO m_blog (user_id, title, description, created) VALUES ({}, {}, {}, {})",
		3, 
		gbkToUtf8(R"('测试中文标题')"), 
		gbkToUtf8(R"('测试中文描述')"),
		R"('2023-01-13 12:39:39')");

	if (0 != mysql_real_query(mysql, query.c_str(), query.size())) {
		std::cout << std::format("mysql_real_query error: {}.\n", mysql_error(mysql));
		return;
	}

	std::cout << std::format("Insert affected rows: {}.\n", mysql_affected_rows(mysql));
}

void test_update(MYSQL* mysql) {
	std::string query = std::format("UPDATE m_blog SET description = {} WHERE title = {}",
		gbkToUtf8(R"('这是一个新描述')"),
		gbkToUtf8(R"('测试中文标题')")
		);

	if (0 != mysql_real_query(mysql, query.c_str(), query.size())) {
		std::cout << std::format("mysql_real_query error: {}.\n", mysql_error(mysql));
		return;
	}

	std::cout << std::format("Update affected rows: {}.\n", mysql_affected_rows(mysql));
}

void test_delete(MYSQL* mysql) {
	std::string query = std::format("DELETE FROM m_blog WHERE title = {}", gbkToUtf8(R"('测试中文标题')"));

	if (0 != mysql_real_query(mysql, query.c_str(), query.size())) {
		std::cout << std::format("mysql_real_query error: {}.\n", mysql_error(mysql));
		return;
	}

	std::cout << std::format("Delete affected rows: {}.\n", mysql_affected_rows(mysql));
}

int main() {
	MYSQL *mysql = nullptr; 

	do {
		if (nullptr == (mysql = mysql_init(mysql))) {
			std::cout << std::format("mysql_init error: {}.\n", mysql_error(mysql));
			break;
		}

		if (nullptr == mysql_real_connect(mysql, "localhost", "whoknows", "idoesnottknow", "youguess", 3306, nullptr, 0)) {
			std::cout << std::format("mysql_real_connect error: {}.\n", mysql_error(mysql));
			break;
		}

		if (0 != mysql_set_character_set(mysql, "utf8")) {
			std::cout << std::format("mysql_set_character_set error: {}.\n", mysql_error(mysql));
			break;
		}

		//test_query(mysql);
		//test_insert(mysql);
		//test_update(mysql);
		test_delete(mysql);
	} while (0);

	if (nullptr != mysql) {
		mysql_close(mysql);
	}

	return 0;
}
