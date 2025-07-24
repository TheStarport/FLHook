#pragma once

#include <bsoncxx/builder/basic/document.hpp>
#include <bsoncxx/builder/basic/array.hpp>
#include <bsoncxx/document/view.hpp>

#define B_KVP  bsoncxx::builder::basic::kvp
#define B_MARR bsoncxx::builder::basic::make_array
#define B_MDOC bsoncxx::builder::basic::make_document

#define B_ARR  bsoncxx::builder::basic::array
#define B_DOC  bsoncxx::builder::basic::document

#define B_VIEW bsoncxx::document::view
#define B_VAL  bsoncxx::document::value
#define B_VOV  bsoncxx::document::view_or_value
