/*
 *
 *  D-Bus++ - C++ bindings for D-Bus
 *
 *  Copyright (C) 2005-2007  Paolo Durante <shackan@gmail.com>
 *
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2.1 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <fstream>

#include "xml2cpp.h"
#include "generate_adaptor.h"
#include "generate_proxy.h"

using namespace DBus;

const char *xml_file = nullptr;
const char *proxy_file = nullptr;
const char *adaptor_file = nullptr;

int usage(const char *prog_name)
{
  fprintf(stderr, "%s XML_FILE [OPTIONS...]\n\n"
                  "dbus-c++ XML to .cpp converter\n\n"
                  "  --proxy FILE    Generate proxy file\n"
                  "  --adaptor FILE  Generate adaptor file\n"
                  "  -h --help       Print help and exit\n"
                  "  -V --version    Print version and exit\n"
                , prog_name);
  return 0;
}

int version(const char *prog_name)
{
  printf("%s 0.1\n", prog_name);
  return 0;
}

int parse_argv(int argc, char *argv[])
{
  enum optid { PROXY=0x100, ADAPTOR };
  const struct option options[] = {
    {"proxy",   required_argument, nullptr, PROXY},
    {"adaptor", required_argument, nullptr, ADAPTOR},
    {"help",    no_argument,       nullptr, 'h'},
    {"version", no_argument,       nullptr, 'V'},
    {}
  };
  int c;
  while ((c = getopt_long(argc, argv, "hV", options, nullptr)) >= 0) {
    switch (c) {
      case 'h':
        return usage(argv[0]);
      case 'V':
        return version(argv[0]);
      case PROXY:
        proxy_file = optarg;
        break;
      case ADAPTOR:
        adaptor_file = optarg;
        break;
      default:
        return -1;
    }
  }
  return 1;
}

int main(int argc, char *argv[])
{
  if (argc < 2) {
    fprintf(stderr, "no file specified (see --help)\n");
    return 1;
  }
  int r = parse_argv(argc, argv);
  if (r <= 0)
    return r < 0 ? 1 : 0;
  xml_file = argv[optind];
  std::ifstream xmlfile(xml_file);
  if (xmlfile.bad()) {
    fprintf(stderr, "unable to open file %s\n", xml_file);
    return 1;
  }

  Xml::Document doc;
  try {
    xmlfile >> doc;
  } catch (Xml::Error &e) {
    fprintf(stderr, "error parsing %s: %s\n", xml_file, e.what());
    return -1;
  }

  if (!doc.root) {
    fputs("empty document", stderr);
    return -1;
  }

  if (proxy_file != nullptr)
    generate_proxy(doc, proxy_file);
  if (adaptor_file != nullptr)
    generate_adaptor(doc, adaptor_file);

  return 0;
}
