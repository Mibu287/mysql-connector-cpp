/*
 * Copyright (c) 2015, 2018, Oracle and/or its affiliates. All rights reserved.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License, version 2.0, as
 * published by the Free Software Foundation.
 *
 * This program is also distributed with certain software (including
 * but not limited to OpenSSL) that is licensed under separate terms,
 * as designated in a particular file or component or in included license
 * documentation.  The authors of MySQL hereby grant you an
 * additional permission to link the program and your derivative works
 * with the separately licensed software that they have included with
 * MySQL.
 *
 * Without limiting anything contained in the foregoing, this file,
 * which is part of MySQL Connector/C++, is also subject to the
 * Universal FOSS Exception, version 1.0, a copy of which can be found at
 * http://oss.oracle.com/licenses/universal-foss-exception.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License, version 2.0, for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation, Inc.,
 * 51 Franklin St, Fifth Floor, Boston, MA 02110-1301  USA
 */

#include "json_parser.h"
#include <mysql/cdk.h>
PUSH_SYS_WARNINGS_CDK
#include "rapidjson/reader.h"
#include "rapidjson/error/en.h"
POP_SYS_WARNINGS_CDK
#include <stack>


PUSH_SYS_WARNINGS_CDK
#include <stdlib.h>
POP_SYS_WARNINGS_CDK


/*
  The maximum absolute value that 64-bit signed integer can have.
  For most platforms it is the minimum negative LLONG_MIN.
  The absolute value for it cannot be obtained because
  ABS(LLONG_MAX) < ABS(LLONG_MIN). Therefore, we define a constant.
*/
#define INTEGER_ABS_MAX 9223372036854775808UL

using namespace parser;
using cdk::string;
using cdk::JSON;
typedef  cdk::JSON::Processor Processor;

void JSON_parser::process(Expr_base::Processor &prc) const
{
  rapidjson::Reader m_parser;
  rapidjson::InsituStringStream ss(const_cast<char*>(m_json.data()));

  /*
    struck to be used has handler on rapidjson parser
  */
  struct Processor_cvt
  {

    struct processors
    {
      Processor *m_obj = nullptr;
      Processor::Any_prc* m_key = nullptr;
      Processor::Any_prc::List_prc* m_arr = nullptr;

      processors(Processor *obj)
      {
        m_obj = obj;
      }

      processors(Processor::Any_prc* key)
      {
        m_key = key;
      }

      processors(Processor::Any_prc::List_prc* arr)
      {
        m_arr = arr;
      }
    };

    std::stack<processors> m_stack;

    Processor_cvt (Processor &prc)
    {
      m_stack.push(&prc);
    }

    bool Null()
    {
      if (m_stack.empty())
        return false;

      if (m_stack.top().m_key)
      {
        m_stack.top().m_key->scalar()->null();
        m_stack.pop();
      }
      else if (m_stack.top().m_arr)
        m_stack.top().m_arr->list_el()->scalar()->null();
      return true;
    }

    bool Bool(bool b)
    {
      if (m_stack.empty())
        return false;


      if (m_stack.top().m_key)
      {
        m_stack.top().m_key->scalar()->yesno(b);
        m_stack.pop();
      }
      else if (m_stack.top().m_arr)
        m_stack.top().m_arr->list_el()->scalar()->yesno(b);
      else
        return false;

      return true;
    }

    bool Int(int i)
    {
      if (m_stack.empty())
        return false;


      if (m_stack.top().m_key)
      {
        m_stack.top().m_key->scalar()->num(static_cast<int64_t>(i));
        m_stack.pop();
      }
      else if (m_stack.top().m_arr)
        m_stack.top().m_arr->list_el()->scalar()->num(static_cast<int64_t>(i));
      else
        return false;

      return true;
    }

    bool Uint(unsigned u)
    {
      if (m_stack.empty())
        return false;


      if (m_stack.top().m_key)
      {
        m_stack.top().m_key->scalar()->num(static_cast<uint64_t>(u));
        m_stack.pop();
      }
      else if (m_stack.top().m_arr)
        m_stack.top().m_arr->list_el()->scalar()->num(static_cast<uint64_t>(u));
      else
        return false;

      return true;
    }

    bool Int64(int64_t i)
    {
      if (m_stack.empty())
        return false;


      if (m_stack.top().m_key)
      {
        m_stack.top().m_key->scalar()->num(i);
        m_stack.pop();
      }
      else if (m_stack.top().m_arr)
        m_stack.top().m_arr->list_el()->scalar()->num(i);
      else
        return false;

      return true;
    }

    bool Uint64(uint64_t u)
    {
      if (m_stack.empty())
        return false;


      if (m_stack.top().m_key)
      {
        m_stack.top().m_key->scalar()->num(u);
        m_stack.pop();
      }
      else if (m_stack.top().m_arr)
        m_stack.top().m_arr->list_el()->scalar()->num(u);
      else
        return false;
      return true;
    }

    bool Double(double d)
    {
      if (m_stack.empty())
        return false;


      if (m_stack.top().m_key)
      {
        m_stack.top().m_key->scalar()->num(d);
        m_stack.pop();
      }
      else if (m_stack.top().m_arr)
        m_stack.top().m_arr->list_el()->scalar()->num(d);
      else
        return false;
      return true;
    }

    bool RawNumber(const char* /*str*/,
                   rapidjson::SizeType /*length*/,
                   bool /*copy*/)
    {
      // not needed
        return false;
    }

    bool String(const char* str, rapidjson::SizeType length, bool /*copy*/)
    {
      if (m_stack.empty())
        return false;


      if (m_stack.top().m_key)
      {
        m_stack.top().m_key->scalar()->str(std::string(str, length));
        m_stack.pop();
      }
      else if (m_stack.top().m_arr)
        m_stack.top().m_arr->list_el()->scalar()->str(std::string(str, length));

      return true;
    }

    bool Key(const char* str, rapidjson::SizeType length, bool /*copy*/)
    {
      if (m_stack.empty())
        return false;

      if (m_stack.top().m_obj)
        m_stack.push(m_stack.top().m_obj->key_val(std::string(str, length)));
      else
        return false;

      return true;
    }

    bool StartObject()
    {
      if (m_stack.empty())
        return false;

      if(m_stack.top().m_key)
      {
        m_stack.push(m_stack.top().m_key->doc());
      }
      else if (m_stack.top().m_arr)
      {
        m_stack.push(m_stack.top().m_arr->list_el()->doc());
      }
      else if (!m_stack.top().m_obj)
        return false;

      m_stack.top().m_obj->doc_begin();

      return true;
    }


    bool EndObject(rapidjson::SizeType )
    {
      if(m_stack.empty())
        return false;

      if (m_stack.top().m_obj)
      {
        m_stack.top().m_obj->doc_end();
      }

      m_stack.pop(); // Pop obj

      if (!m_stack.empty() && m_stack.top().m_key)
        m_stack.pop(); // Pop key

      return true;
    }
    bool StartArray()
    {
      if (m_stack.empty() )
        return false;

      if (m_stack.top().m_key)
      {
        m_stack.push(m_stack.top().m_key->arr());
      }
      else if (m_stack.top().m_arr)
      {
        m_stack.push(m_stack.top().m_arr->list_el()->arr());
      }
      else return false;

      m_stack.top().m_arr->list_begin();

      return true;
    }

    bool EndArray(rapidjson::SizeType )
    {
      if (m_stack.empty() )
        return false;

      if (!m_stack.top().m_arr)
        return false;

      m_stack.top().m_arr->list_end();
      m_stack.pop();// Pop array
      if (!m_stack.empty() && m_stack.top().m_key)
        m_stack.pop();// pop key

      return true;
    }

  };

  Processor_cvt cvt(prc);

  /*
     kParseInsituFlag is used to reduce memory usage and increase speed.
  */

  auto error = m_parser.Parse<rapidjson::kParseInsituFlag>(ss, cvt);
  if (error.IsError())
  {
    throw JSON_parser::Error(m_json,
                             error.Offset(),
                             rapidjson::GetParseError_En(error.Code()));
  }
}

void json_parse(const string &json, Processor &dp)
{
  JSON_parser    parser(json);
  parser.process(dp);
}



//  Clasify a token as a JSON token.

JSON_token_base::Token_type JSON_token_base::get_jtype(const Token &tok)
{
  switch (tok.get_type())
  {
  case Token::PLUS:
    return JSON_token_base::PLUS;
  case Token::MINUS:
    return JSON_token_base::MINUS;
  case Token::NUMBER:
    return JSON_token_base::NUMBER;
  case Token::INTEGER:
    return JSON_token_base::INTEGER;

  // TODO: According to JSON specs only double-quote strings are allowed?
  case Token::QQSTRING:
  case Token::QSTRING:
    return JSON_token_base::STRING;

  case Token::WORD:
    {
      // TODO: Are these JSON literals case sensitivie?
      const string &id = tok.get_text();
      if (id == L"null") return JSON_token_base::T_NULL;
      if (id == L"true") return JSON_token_base::T_TRUE;
      if (id == L"false") return JSON_token_base::T_FALSE;
    }

  default:
    return JSON_token_base::OTHER;
  }
}



bool JSON_scalar_parser::do_parse(Processor *vp)
{
  if (!tokens_available())
    return false;

  bool neg = false;

  Token tok = *consume_token();
  Token_type tt = get_jtype(tok);

  switch (tt)
  {
  case STRING:
    if(vp)
      vp->str(tok.get_text());
    return true;

  case MINUS:
  case PLUS:
    neg = (MINUS == tt);
    if (!tokens_available())
      parse_error(L"Expected number after +/- sign");
    tok = *consume_token();
    tt = get_jtype(tok);
    break;

  case T_NULL:
    if (vp)
      vp->null();
    return true;

  case T_TRUE:
  case T_FALSE:
    if (vp)
      vp->yesno(T_TRUE == tt);
    return true;

  default:
    // if none of the above, then it should be a number
    break;
  }

  // Numeric value

  switch (tt)
  {
  case NUMBER:
    {
      if(vp)
      {
        double val = strtod(tok.get_text());
        vp->num(neg ? -val : val);
      }
      return true;
    }

  case INTEGER:
    try {

      if(vp)
      {
        // TODO: Is this logic right? Should we report only negative values
        // as signed integers?

        uint64_t val = strtoui(tok.get_text());
        if (val > INTEGER_ABS_MAX)
        {
          if (neg)
            parse_error(L"Numeric value is too large for a signed type");
          // Unsigned type is only returned for large values
          vp->num(val);
        }
        else
        {
          // Absolute values of 9223372036854775808UL can only be negative
          if (!neg && val == INTEGER_ABS_MAX)
            parse_error(L"Numeric value is too large for a signed type");
          // All values ABS(val) < 9223372036854775808UL are treated as signed
          vp->num(neg ? -(int64_t)val : (int64_t)val);
        }
      }

      return true;
    }
    catch (const Numeric_conversion_error &e)
    {
      parse_error(e.msg());
    }

  default:
    parse_error(L"Invalid JSON value");
    return false; // quiet compile warnings
  }
}


