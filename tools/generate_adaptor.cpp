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

#include <iostream>
#include <fstream>
#include <cstdlib>
#include <algorithm>

#include "generator_utils.h"
#include "generate_adaptor.h"

using namespace DBus;

extern const char *tab;
extern const char *header;
extern const char *dbus_includes;

/*! Generate adaptor code for a XML introspection
  */
void generate_adaptor(const Xml::Document &doc, const char *filename)
{
  std::ostringstream body;
  std::ostringstream head;
  std::vector<std::string> include_vector;

  head << header;
  std::string filestring = filename;
  underscorize(filestring);

  const auto cond_comp = "__dbusxx__" + filestring + "__ADAPTOR_MARSHAL_H\n";

  head << "#ifndef " << cond_comp
       << "#define " << cond_comp
       << dbus_includes;

  // iterate over all interface definitions
  for (const auto& i : (*doc.root)["interface"]) {
    const auto& iface = *i;
    const auto methods = iface["method"];
    const auto signals = iface["signal"];
    const auto properties = iface["property"];
    Xml::Nodes ms;
    ms.insert(ms.end(), methods.begin(), methods.end());
    ms.insert(ms.end(), signals.begin(), signals.end());

    // gets the name of a interface: <interface name="XYZ">
    const auto ifacename = iface.get("name");

    // these interface names are skipped.
    if (ifacename == "org.freedesktop.DBus.Introspectable"
        || ifacename == "org.freedesktop.DBus.Properties") {
      std::cerr << "skipping interface " << ifacename << '\n';
      continue;
    }

    std::istringstream ss(ifacename);
    std::string nspace;
    unsigned int nspaces = 0;

    // prints all the namespaces defined with <interface name="X.Y.Z">
    while (ss.str().find('.', ss.tellg()) != std::string::npos) {
      std::getline(ss, nspace, '.');
      body << "namespace " << nspace << " {\n";
      ++nspaces;
    }
    body << '\n';

    std::string ifaceclass;
    std::getline(ss, ifaceclass);

    // a "_adaptor" is added to class name to distinguish between proxy and adaptor
    ifaceclass += "_adaptor";

    std::cerr << "generating code for interface " << ifacename << "...\n";

    // the code from class definiton up to opening of the constructor is generated...
    body << "class " << ifaceclass << '\n'
         << ": public ::DBus::InterfaceAdaptor\n"
         << "{\n"
         << "public:\n\n"
         << tab << ifaceclass << "()\n"
         << tab << ": ::DBus::InterfaceAdaptor(\"" << ifacename << "\")\n"
         << tab << "{\n";

    // generates code to bind the properties
    for (const auto& pi : properties) {
      auto& property = *pi;
      body << tab << tab << "bind_property("
           << property.get("name") << ", "
           << "\"" << property.get("type") << "\", "
           << std::boolalpha
           << (property.get("access").find("read") != std::string::npos)
           << ", "
           << std::boolalpha
           << (property.get("access").find("write") != std::string::npos)
           << ");\n";
    }

    // generate code to register all methods
    for (const auto& mi : methods) {
      auto& method = *mi;
      body << tab << tab << "register_method("
           << ifaceclass << ", " << method.get("name") << ", "
           << stub_name(method.get("name")) << ");\n";
    }

    body << tab << "}\n\n"
         << tab << "::DBus::IntrospectedInterface *introspect() const\n"
         << tab << "{\n";

    // generate the introspect arguments
    for (const auto& mi : ms) {
      auto& method = *mi;

      body << tab << tab << "static ::DBus::IntrospectedArgument "
           << method.get("name") << "_args[] =\n"
           << tab << tab << "{\n";

      auto args = method["arg"];
      for (const auto& ai : args) {
        body << tab << tab << tab << "{ ";

        auto& arg = *ai;
        const auto arg_name = arg.get("name");
        if (!arg_name.empty())
          body << "\"" << arg_name << "\", ";
        else
          body << "0, ";
        body << "\"" << arg.get("type") << "\", "
             << std::boolalpha << (arg.get("direction") == "in")
             << " },\n";
      }
      body << tab << tab << tab << "{ 0, 0, 0 }\n"
           << tab << tab << "};\n";
    }

    body << tab << tab << "static ::DBus::IntrospectedMethod "
         << ifaceclass << "_methods[] = \n"
         << tab << tab << "{\n";

    // generate the introspect methods
    for (const auto& mi : methods) {
      const auto& method = *mi;
      body << tab << tab << tab
           << "{ \"" << method.get("name") << "\", "
           << method.get("name") << "_args },\n";
    }

    body << tab << tab << tab << "{ 0, 0 }\n"
         << tab << tab << "};\n"
         << tab << tab
         << "static ::DBus::IntrospectedMethod " << ifaceclass
         << "_signals[] = \n"
         << tab << tab << "{\n";

    for (const auto& si : signals) {
      const auto& method = *si;
      body << tab << tab << tab << "{ \""
           << method.get("name") << "\", " << method.get("name") << "_args },\n";
    }

    body << tab << tab << tab << "{ 0, 0 }\n"
         << tab << tab << "};\n"
         << tab << tab << "static ::DBus::IntrospectedProperty "
         << ifaceclass << "_properties[] = \n"
         << tab << tab << "{\n";

    for (const auto& pi : properties) {
      const auto& property = *pi;
      body << tab << tab << tab << "{ "
           << "\"" << property.get("name") << "\", "
           << "\"" << property.get("type") << "\", "
           << std::boolalpha
           << (property.get("access").find("read") != std::string::npos)
           << ", "
           << std::boolalpha
           << (property.get("access").find("write") != std::string::npos)
           << " },\n";
    }


    body << tab << tab << tab << "{ 0, 0, 0, 0 }\n"
         << tab << tab << "};\n"
    // generate the Introspected interface
         << tab << tab<< "static ::DBus::IntrospectedInterface "
         << ifaceclass << "_interface = \n"
         << tab << tab << "{\n"
         << tab << tab << tab << "\"" << ifacename << "\",\n"
         << tab << tab << tab << ifaceclass << "_methods,\n"
         << tab << tab << tab << ifaceclass << "_signals,\n"
         << tab << tab << tab << ifaceclass << "_properties\n"
         << tab << tab << "};\n"
         << tab << tab << "return &" << ifaceclass << "_interface;\n"
         << tab << "}\n\n"
         << "public:\n\n"
         << tab << "/* properties exposed by this interface, use\n"
         << tab << " * property() and property(value) to get and set a particular property\n"
         << tab << " */\n";

    // generate the properties code
    for (const auto& pi : properties) {
      const auto& property = *pi;
      body << tab
           << "::DBus::PropertyAdaptor< "
           << signature_to_type(property.get("type"))
           << " > "
           << property.get("name") << ";\n";
    }

    body << "\npublic:\n\n"
         << tab << "/* methods exported by this interface,\n"
         << tab << " * you will have to implement them in your ObjectAdaptor\n"
         << tab << " */\n";

    // generate the methods code
    for (const auto& mi : methods) {
      const auto& method = *mi;
      const auto args = method["arg"];
      std::string arg_object;

      const auto annotations_object = args["annotation"].select(
          "name", "org.freedesktop.DBus.Object");
      if (!annotations_object.empty())
        arg_object = annotations_object.front()->get("value");

      body << tab << "virtual ";

      const auto args_out = args.select("direction", "out");
      // return type is 'void' if none or multible return values
      if (args_out.empty() || args_out.size() > 1)
        body << "void ";
      else if (args_out.size() == 1) {
        // generate basic or object return type
        if (!arg_object.empty())
          body << arg_object << " ";
        else
          body << signature_to_type(args_out.front()->get("type")) << " ";
      }

      // generate the method name
      body << method.get("name") << "(";

      // generate the methods 'in' variables
      unsigned int i = 0;
      const auto args_in = args.select("direction", "in");
      for (auto ai = args_in.begin(); ai != args_in.end(); ++ai, ++i) {
        const auto& arg = **ai;
        std::string arg_object;

        const auto annotations_object = arg["annotation"].select(
            "name", "org.freedesktop.DBus.Object");
        if (!annotations_object.empty())
          arg_object = annotations_object.front()->get("value");

        // generate basic signature only if no object name available...
        if (arg_object.empty())
          body << "const " << signature_to_type(arg.get("type")) << "& ";
        // ...or generate object style if available
        else {
          body << "const " << arg_object << "& ";

          // store a object name to later generate header includes
          include_vector.push_back(arg_object);
        }

        const auto arg_name = arg.get("name");
        if (!arg_name.empty())
          body << arg_name;

        if (i + 1 != args_in.size() || args_out.size() > 1)
          body << ", ";
      }

      // generate the method 'out' variables if multibe 'out' values exist
      if (args_out.size() > 1) {
        unsigned int i = 0;
        for (auto ao = args_out.begin(); ao != args_out.end(); ++ao, ++i) {
          const auto& arg = **ao;
          std::string arg_object;

          const auto annotations_object = arg["annotation"].select(
              "name", "org.freedesktop.DBus.Object");
          if (!annotations_object.empty())
            arg_object = annotations_object.front()->get("value");

          // generate basic signature only if no object name available...
          if (arg_object.empty())
            body << signature_to_type(arg.get("type")) << "& ";
          // ...or generate object style if available
          else {
            body << arg_object << "& ";

            // store a object name to later generate header includes
            include_vector.push_back(arg_object);
          }

          const auto arg_name = arg.get("name");
          if (!arg_name.empty())
            body << arg_name;

          if (i + 1 != args_out.size())
            body << ", ";
        }
      }
      body << ") = 0;\n";
    }

    body << "\npublic:\n\n"
         << tab << "/* signal emitters for this interface\n"
         << tab << " */\n";

    // generate the signals code
    for (const auto& si : signals) {
      const auto& signal = *si;
      const auto args = signal["arg"];

      body << tab << "void " << signal.get("name") << "(";

      // generate the signal arguments
      unsigned int i = 0;
      for (const auto a : args) {
        const auto& arg = *a;
        const auto annotations_object = arg["annotation"].select(
            "name", "org.freedesktop.DBus.Object");
        std::string arg_object;

        if (!annotations_object.empty())
          arg_object = annotations_object.front()->get("value");

        // generate basic signature only if no object name available...
        if (arg_object.empty())
          body << "const "
               << signature_to_type(arg.get("type")) << "& arg" << i + 1;
        // ...or generate object style if available
        else {
          body << "const " << arg_object << "& arg" << i + 1;

          // store a object name to later generate header includes
          include_vector.push_back(arg_object);
        }

        if (i + 1 != args.size())
          body << ", ";

        ++i;
      }

      body << ")\n"
           << tab << "{\n"
           << tab << tab
           << "::DBus::SignalMessage sig(\"" << signal.get("name") << "\");\n";

      // generate the signal body
      if (!args.empty()) {
        body << tab << tab << "::DBus::MessageIter wi = sig.writer();\n";

        unsigned int i = 0;
        for (auto a = args.begin(); a != args.end(); ++a, ++i) {
          const auto& arg = **a;
          std::string arg_object;

          const auto annotations_object = arg["annotation"].select(
              "name", "org.freedesktop.DBus.Object");
          if (!annotations_object.empty())
            arg_object = annotations_object.front()->get("value");

          if (!arg_object.empty()) {
            body << tab << tab
                 << signature_to_type(arg.get("type")) << " _arg" << i + 1
                 << ";\n"
                 << tab << tab << "_arg" << i + 1 << " << arg" << i + 1
                 << ";\n"
                 << tab << tab << "wi << _arg" << i + 1 << ";\n";
          } else
            body << tab << tab << "wi << arg" << i + 1 << ";\n";
        }
      }

      // emit the signal in method body
      body << tab << tab << "emit_signal(sig);\n"
           << tab << "}\n";
    }

    body << "\nprivate:\n\n"
         << tab << "/* unmarshalers (to unpack the DBus message before calling the actual interface method)\n"
         << tab << " */\n";

    // generate the unmarshalers
    for (const auto& mi : methods) {
      const auto& method = *mi;
      const auto args = method["arg"];
      const auto args_in = args.select("direction", "in");
      const auto args_out = args.select("direction", "out");

      body << tab << "::DBus::Message " << stub_name(method.get("name"))
           << "(const ::DBus::CallMessage &call)\n"
           << tab << "{\n";
      if(!args_in.empty())
         body << tab << tab << "::DBus::MessageIter ri = call.reader();\n\n";

      // generate the 'in' variables
      unsigned int i = 1;
      for (auto ai = args_in.begin(); ai != args_in.end(); ++ai, ++i)
        body << tab << tab << signature_to_type((**ai).get("type"))
             << " argin" << i << "; ri >> argin" << i << ";\n";

      // generate the 'in' object variables
      i = 1;
      for (auto ai = args_in.begin(); ai != args_in.end(); ++ai, ++i)
      {
        const auto& arg = **ai;
        const auto annotations_object = arg["annotation"].select(
            "name", "org.freedesktop.DBus.Object");
        std::string arg_object;

        if (!annotations_object.empty())
          arg_object = annotations_object.front()->get("value");

        if (!arg_object.empty())
          body << tab << tab << arg_object << " _argin" << i
               << "; _argin" << i << " << argin" << i << ";\n";
      }

      // generate 'out' variables
      if (!args_out.empty()) {
        unsigned int i = 1;
        for (auto ao = args_out.begin(); ao != args_out.end(); ++ao, ++i)
          body << tab << tab
               << signature_to_type((**ao).get("type")) << " argout" << i
               // detect if one or more 'out' parameters will be handled
               << (args_out.size() == 1 ? " = " : ";\n");
      }

      // generate 'out' object variables
      if (!args_out.empty()) {
        unsigned int i = 1;
        for (auto ao = args_out.begin(); ao != args_out.end(); ++ao, ++i) {
          const auto& arg = **ao;
          std::string arg_object;

          const auto annotations_object = arg["annotation"].select(
              "name", "org.freedesktop.DBus.Object");
          if (!annotations_object.empty())
            arg_object = annotations_object.front()->get("value");

          // generate object types
          if (!arg_object.empty())
            body << tab << tab << arg_object << " _argout" << i << ";\n";
        }
      }

      // generate in '<<' operation
      i = 0;
#if 0
      for (auto ai = args_in.begin(); ai != args_in.end(); ++ai, ++i)
      {
        auto& arg = **ai;
        string arg_object;

        auto annotations_object = arg["annotation"].select(
            "name", "org.freedesktop.DBus.Object");
        if (!annotations_object.empty())
          arg_object = annotations_object.front()->get("value");
      }
#endif

      // do correct indent
      if (args_out.size() != 1)
        body << tab << tab;

      body << method.get("name") << "(";

      // generate call stub parameters
      i = 0;
      for (auto ai = args_in.begin(); ai != args_in.end(); ++ai, ++i) {
        const auto& arg = **ai;
        std::string arg_object;

        const auto annotations_object = arg["annotation"].select(
            "name", "org.freedesktop.DBus.Object");
        if (!annotations_object.empty())
          arg_object = annotations_object.front()->get("value");

        body << (!arg_object.empty() ? "_argin" : "argin") << i + 1;

        if (i + 1 != args_in.size() || args_out.size() > 1)
          body << ", ";
      }

      if (args_out.size() > 1) {
        i = 0;
        for (auto ao = args_out.begin(); ao != args_out.end(); ++ao, ++i) {
          const auto& arg = **ao;
          std::string arg_object;

          const auto annotations_object = arg["annotation"].select(
              "name", "org.freedesktop.DBus.Object");
          if (!annotations_object.empty())
            arg_object = annotations_object.front()->get("value");

          body << (!arg_object.empty() ? "_argout" : "argout") << i + 1;

          if (i + 1 != args_out.size())
            body << ", ";
        }
      }

      body << ");\n"
           << tab << tab << "::DBus::ReturnMessage reply(call);\n";

      if (!args_out.empty())
      {
        body << tab << tab << "::DBus::MessageIter wi = reply.writer();\n";

        // generate out '<<' operation
        i = 0;
        for (auto ao = args_out.begin(); ao != args_out.end(); ++ao, ++i) {
          const auto& arg = **ao;
          std::string arg_object;

          const auto annotations_object = arg["annotation"].select(
              "name", "org.freedesktop.DBus.Object");
          if (!annotations_object.empty())
            arg_object = annotations_object.front()->get("value");

          if (!arg_object.empty())
            body << tab << tab
                 << "argout" << i + 1
                 << " << _argout" << i + 1 << ";\n";
        }

        for (unsigned int i = 0; i < args_out.size(); ++i)
          body << tab << tab << "wi << argout" << i + 1 << ";\n";
      }

      body << tab << tab << "return reply;\n"
           << tab << "}\n";
    }

    body << "};\n\n";

    for (unsigned int i = 0; i < nspaces; ++i)
      body << "} ";
    body << '\n';
  }

  body << "#endif //" << cond_comp;

  // remove all duplicates in the header include vector
  const auto vec_end_it = std::unique(begin(include_vector), end(include_vector));
  for (auto inc_it = include_vector.begin(); inc_it != vec_end_it; ++inc_it)
    head << "#include \"" << *inc_it << ".h\"\n";
  head << '\n';

  std::ofstream file(filename);
  if (file.bad()) {
    std::cerr << "unable to write file " << filename << '\n';
    exit(-1);
  }

  file << head.str()
       << body.str();

  file.close();
}
