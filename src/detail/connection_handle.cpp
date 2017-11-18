/*
 * Copyright (c) 2013 - 2015, Roland Bock
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 *   Redistributions of source code must retain the above copyright notice, this
 *   list of conditions and the following disclaimer.
 *
 *   Redistributions in binary form must reproduce the above copyright notice, this
 *   list of conditions and the following disclaimer in the documentation and/or
 *   other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include "connection_handle.h"
#include <ciso646>
#include <iostream>
#include <sqlpp11/exception.h>
#include <sqlpp11/mysql/connection_config.h>

namespace sqlpp
{
  namespace mysql
  {
    namespace detail
    {
      void handle_cleanup(MYSQL* handle)
      {
        mysql_close(handle);
        delete handle;
      }

      connection_handle_t::connection_handle_t(const std::shared_ptr<connection_config>& conf)
          : config(conf), mysql(new MYSQL, handle_cleanup)
      {
        if (not mysql_init(mysql.get()))
        {
          throw sqlpp::exception("MySQL: could not init connection data structure");
        }

        if (config->auto_reconnect)
        {
          my_bool my_true = true;
          if (mysql_options(mysql.get(), MYSQL_OPT_RECONNECT, &my_true))
          {
            throw sqlpp::exception("MySQL: could not set option MYSQL_OPT_RECONNECT");
          }
        }

        if (!mysql_real_connect(mysql.get(), config->host.empty() ? nullptr : config->host.c_str(),
                                config->user.empty() ? nullptr : config->user.c_str(),
                                config->password.empty() ? nullptr : config->password.c_str(), nullptr, config->port,
                                config->unix_socket.empty() ? nullptr : config->unix_socket.c_str(),
                                config->client_flag))
        {
          throw sqlpp::exception("MySQL: could not connect to server: " + std::string(mysql_error(mysql.get())));
        }
      }

      connection_handle_t::~connection_handle_t()
      {
      }

      bool connection_handle_t::is_valid()
      {
        return mysql_ping(mysql.get()) == 0;
      }

      void connection_handle_t::reconnect()
      {
        if (!mysql_real_connect(mysql.get(), config->host.empty() ? nullptr : config->host.c_str(),
                                config->user.empty() ? nullptr : config->user.c_str(),
                                config->password.empty() ? nullptr : config->password.c_str(), nullptr, config->port,
                                config->unix_socket.empty() ? nullptr : config->unix_socket.c_str(),
                                config->client_flag))
        {
          throw sqlpp::exception("MySQL: could not connect to server: " + std::string(mysql_error(mysql.get())));
        }
      }
    }
  }
}
