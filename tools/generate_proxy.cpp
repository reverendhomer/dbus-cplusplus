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
#include "generate_proxy.h"

using namespace std;
using namespace DBus;

extern const char *tab;
extern const char *header;
extern const char *dbus_includes;

/*! Generate proxy code for a XML introspection
  */
void generate_proxy(const Xml::Document &doc, const char *filename)
{
  ostringstream body;
  ostringstream head;
  vector <string> include_vector;

  head << header;
  string filestring = filename;
  underscorize(filestring);

  string cond_comp = "__dbusxx__" + filestring + "__PROXY_MARSHAL_H\n";

  head << "#ifndef " << cond_comp
       << "#define " << cond_comp
       << dbus_includes;

  // iterate over all interface definitions
  for (const auto& i : (*doc.root)["interface"]) {
    auto& iface = *i;
    auto methods = iface["method"];
    auto signals = iface["signal"];
    Xml::Nodes ms;
    ms.insert(ms.end(), methods.begin(), methods.end());
    ms.insert(ms.end(), signals.begin(), signals.end());

    // gets the name of a interface: <interface name="XYZ">
    auto ifacename = iface.get("name");
    if (ifacename == "org.freedesktop.DBus.Introspectable"
        || ifacename == "org.freedesktop.DBus.Properties") {
      cerr << "skipping interface " << ifacename << endl;
      continue;
    }

    istringstream ss(ifacename);
    string nspace;
    unsigned int nspaces = 0;

    // prints all the namespaces defined with <interface name="X.Y.Z">
    while (ss.str().find('.', ss.tellg()) != string::npos) {
      getline(ss, nspace, '.');
      body << "namespace " << nspace << " {\n";
      ++nspaces;
    }
    body << endl;

    string ifaceclass;
    getline(ss, ifaceclass);

    // a "_proxy" is added to class name to distinguish between proxy and adaptor
    ifaceclass += "_proxy";

    cerr << "generating code for interface " << ifacename << "...\n";

    // the code from class definiton up to opening of the constructor is generated...
    body << "class " << ifaceclass << '\n'
         << ": public ::DBus::InterfaceProxy\n"
         << "{\n"
         << "public:\n\n"
         << tab << ifaceclass << "()\n"
         << tab << ": ::DBus::InterfaceProxy(\"" << ifacename << "\")\n"
         << tab << "{\n";

    // generates code to connect all the signal stubs; this is still inside the constructor
    for (const auto& si : signals) {
      auto& signal = *si;

      // string marshname = "_" + signal.get("name") + "_stub";

      body << tab << tab << "connect_signal("
           << ifaceclass << ", " << signal.get("name") << ", "
           << stub_name(signal.get("name")) << ");\n";
    }

    // the constructor ends here
    body << tab << "}\n\n"
    // write public block header for properties
         << "public:\n\n"
         << tab << "/* properties exported by this interface */\n";

    // this loop generates all properties
    for (const auto& pi : iface["property"]) {
      auto& property = *pi;
      auto prop_name = property.get("name");
      auto property_access = property.get("access");
      if (property_access == "read" || property_access == "readwrite") {
        body << tab << tab
             << "const "<< signature_to_type(property.get("type"))
             << " " << prop_name << "() {\n"
             << tab << tab << tab << "::DBus::CallMessage call;\n "
             << tab << tab << tab
             << "call.member(\"Get\");"
             << " call.interface(\"org.freedesktop.DBus.Properties\");\n"
             << tab << tab << tab
             << "::DBus::MessageIter wi = call.writer();\n"
             << tab << tab << tab
             << "const std::string interface_name = \"" << ifacename << "\";\n"
             << tab << tab << tab
             << "const std::string property_name  = \"" << prop_name << "\";\n"
             << tab << tab << tab << "wi << interface_name;\n"
             << tab << tab << tab << "wi << property_name;\n"
             << tab << tab << tab
             << "::DBus::Message ret = this->invoke_method (call);\n"
        // TODO: support invoke_method_NoReply for properties
             << tab << tab << tab
             << "::DBus::MessageIter ri = ret.reader();\n"
             << tab << tab << tab << "::DBus::Variant argout;\n"
             << tab << tab << tab << "ri >> argout;\n"
             << tab << tab << tab << "return argout;\n"
             << tab << tab << "};\n";
      }

      if (property_access == "write" || property_access == "readwrite") {
        body << tab << tab
             << "void " << prop_name << "( const "
             << signature_to_type(property.get("type"))
             << " & input" << ") {\n"
             << tab << tab << tab
             << "::DBus::CallMessage call;\n "
             << tab << tab << tab
             << "call.member(\"Set\");"
             << "  call.interface( \"org.freedesktop.DBus.Properties\");\n"
             << tab << tab << tab
             << "::DBus::MessageIter wi = call.writer();\n"
             << tab << tab << tab << "::DBus::Variant value;\n"
             << tab << tab << tab << "::DBus::MessageIter vi = value.writer();\n"
             << tab << tab << tab << "vi << input;\n"
             << tab << tab << tab
             << "const std::string interface_name = \"" << ifacename << "\";\n"
             << tab << tab << tab
             << "const std::string property_name  = \"" << prop_name << "\";\n"
             << tab << tab << tab << "wi << interface_name;\n"
             << tab << tab << tab << "wi << property_name;\n"
             << tab << tab << tab << "wi << value;\n"
             << tab << tab << tab
             << "::DBus::Message ret = this->invoke_method (call);\n"
        // TODO: support invoke_method_noreply for properties
             << tab << tab << "};\n";
      }
    }

    // write public block header for methods
    body << "public:\n\n"
         << tab << "/* methods exported by this interface,\n"
         << tab << " * this functions will invoke the corresponding methods on the remote objects\n"
         << tab << " */\n";

    // this loop generates all methods
    for (const auto& mi : methods) {
      auto& method = *mi;
      string arg_object;

      // parse method level noreply annotations
      auto annotations_noreply = method["annotation"].select(
          "name", "org.freedesktop.DBus.Method.NoReply");
      bool annotation_noreply_value = false;
      if (!annotations_noreply.empty()
          && annotations_noreply.front()->get("value") == "true")
        annotation_noreply_value = true;

      auto args = method["arg"];
      auto annotations_object = args["annotation"].select(
          "name", "org.freedesktop.DBus.Object");
      if (!annotations_object.empty())
        arg_object = annotations_object.front()->get("value");

      auto args_out = args.select("direction", "out");
      if (args_out.size() == 0 || args_out.size() > 1)
        body << tab << "void ";
      else if (args_out.size() == 1)
        body << tab
             << (arg_object.empty()
                 ? signature_to_type(args_out.front()->get("type"))
                 : arg_object)
             << " ";

      body << method.get("name") << "(";

      // generate all 'in' arguments for a method signature
      auto args_in = args.select("direction", "in");
      unsigned int i = 0;
      for (auto ai = args_in.begin(); ai != args_in.end(); ++ai, ++i) {
        auto& arg = **ai;
        string arg_object;

        auto annotations_object = arg["annotation"].select(
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

        auto arg_name = arg.get("name");
        if (!arg_name.empty())
          body << arg_name;
        else
          body << "argin" << i;

        if (i + 1 != args_in.size() || args_out.size() > 1)
          body << ", ";
      }

      if (args_out.size() > 1) {
        // generate all 'out' arguments for a method signature
        unsigned int j = 0;
        for (auto ao = args_out.begin(); ao != args_out.end(); ++ao, ++j) {
          auto& arg = **ao;
          string arg_object;

          auto annotations_object = arg["annotation"].select(
              "name", "org.freedesktop.DBus.Object");
          if (!annotations_object.empty())
            arg_object = annotations_object.front()->get("value");

          // generate basic signature only if no object name available...
          if (arg_object.empty())
            body << signature_to_type(arg.get("type")) << "&";
          // ...or generate object style if available
          else {
            body << arg_object << "& ";

            // store a object name to later generate header includes
            include_vector.push_back(arg_object);
          }

          auto arg_name = arg.get("name");
          if (!arg_name.empty())
            body << " " << arg_name;
          else
            body << " argout" << j;

          if (j + 1 != args_out.size())
            body << ", ";
        }
      }
      body << ")\n"
           << tab << "{\n"
           << tab << tab << "::DBus::CallMessage call;\n";

      if (!args_in.empty())
        body << tab << tab << "::DBus::MessageIter wi = call.writer();\n\n";

      // generate all 'in' arguments for a method body
      i = 0;
      for (auto ai = args_in.begin(); ai != args_in.end(); ++ai, ++i) {
        auto& arg = **ai;
        string arg_object;

        auto annotations_object = arg["annotation"].select(
            "name", "org.freedesktop.DBus.Object");
        if (!annotations_object.empty())
          arg_object = annotations_object.front()->get("value");

        auto arg_name = arg.get("name");
        if (arg_name.empty())
          arg_name = "argin" + to_string(i);

        // generate extra code to wrap object
        if (!arg_object.empty()) {
          body << tab << tab
               << signature_to_type(arg.get("type")) << "_" << arg_name << ";\n"
               << tab << tab << "_" << arg_name << " << " << arg_name << ";\n";
          arg_name = string("_") + arg_name;
        }

        body << tab << tab << "wi << " << arg_name << ";\n";
      }

      body << tab << tab << "call.member(\"" << method.get("name") << "\");\n";

      // generate noreply/reply method calls
      if (annotation_noreply_value) {
        if (!args_out.empty()) {
          cerr << "Function: " << method.get("name") << ":\n"
               << "Option 'org.freedesktop.DBus.Method.NoReply' not allowed"
               << " for methods with 'out' variables!\n"
               << "-> Option ignored!\n";

          body << tab << tab << "::DBus::Message ret = invoke_method (call);\n";
        }
        else
          // will only assert in case of no memory
          body << tab << tab << "assert (invoke_method_noreply (call));\n";
      }
      else
        body << tab << tab << "::DBus::Message ret = invoke_method (call);\n";

      if (!args_out.empty())
        body << tab << tab << "::DBus::MessageIter ri = ret.reader();\n\n";

      // generate 'out' values as return if only one existing
      if (args_out.size() == 1) {
        string arg_object;

        auto annotations_object = args_out["annotation"].select(
            "name", "org.freedesktop.DBus.Object");
        if (!annotations_object.empty())
          arg_object = annotations_object.front()->get("value");

        if (!arg_object.empty())
          body << tab << tab << arg_object << " _argout;\n";

        body << tab << tab
             << signature_to_type(args_out.front()->get("type")) << " argout;\n"
             << tab << tab << "ri >> argout;\n";

        if (!arg_object.empty())
          body << tab << tab <<  "_argout << argout;\n"
               << tab << tab << "return _argout;\n";
        else
          body << tab << tab << "return argout;\n";

      } else if (args_out.size() > 1) {
        // generate multible 'out' value
        unsigned int i = 0;
        for (auto ao = args_out.begin(); ao != args_out.end(); ++ao, ++i) {
          auto& arg = **ao;
          string arg_object;

          auto annotations_object = arg["annotation"].select(
              "name", "org.freedesktop.DBus.Object");
          if (!annotations_object.empty())
            arg_object = annotations_object.front()->get("value");

          auto arg_name = arg.get("name");
          if (arg_name.empty())
            arg_name = "argout" + to_string(i);

          if (!arg_object.empty())
            body << tab << tab
                 << signature_to_type(arg.get("type")) << "_" << arg_name << ";\n";

          body << tab << tab << "ri >> "
               << (arg_object.empty() ? "" : "_")
               << arg_name << ";\n";

          if (!arg_object.empty())
            body << tab << tab
                 << arg_name << " << " << "_" << arg_name << ";\n";
        }
      }

      body << tab << "}\n\n";
    }

    // write public block header for signals
    body << "\npublic:\n\n"
         << tab << "/* signal handlers for this interface\n"
         << tab << " */\n";

    // this loop generates all signals
    for (const auto& si : signals) {
      auto& signal = *si;
      auto args = signal["arg"];

      body << tab << "virtual void " << signal.get("name") << "(";

      // this loop generates all argument for a signal
      unsigned int i = 0;
      for (auto ai = args.begin(); ai != args.end(); ++ai, ++i) {
        auto& arg = **ai;
        string arg_object;

        auto annotations_object = arg["annotation"].select(
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

        auto arg_name = arg.get("name");
        if (!arg_name.empty())
          body << arg_name;
        else
          body << "argin" << i;

        if (ai + 1 != args.end())
          body << ", ";
      }
      body << ") = 0;\n";
    }

    // write private block header for unmarshalers
    body << "\nprivate:\n\n"
         << tab << "/* unmarshalers (to unpack the DBus message before calling the actual signal handler)\n"
         << tab << " */\n";

    // generate all the unmarshalers
    for (const auto& si : signals) {
      auto& signal = *si;

      body << tab << "void " << stub_name(signal.get("name"))
           << "(const ::DBus::SignalMessage &sig)\n"
           << tab << "{\n";

      auto args = signal["arg"];
      if (!args.empty())
        body << tab << tab << "::DBus::MessageIter ri = sig.reader();\n\n";

      unsigned int i = 0;
      for (auto ai = args.begin(); ai != args.end(); ++ai, ++i) {
        auto& arg = **ai;
        string arg_object;

        auto annotations_object = arg["annotation"].select(
            "name", "org.freedesktop.DBus.Object");
        if (!annotations_object.empty())
          arg_object = annotations_object.front()->get("value");

        body << tab << tab << signature_to_type(arg.get("type")) << " ";

        // use a default if no arg name given
        auto arg_name = arg.get("name");
        if (arg_name.empty())
          arg_name = "arg" + to_string(i);

        body << arg_name << ";\n"
             << tab << tab << "ri >> " << arg_name << ";\n";

        // if a object type is used create a local variable and insert values with '<<' operation
        if (!arg_object.empty()) {
          body << tab << tab << arg_object << " _" << arg_name << ";\n"
               << tab << tab << "_" << arg_name << " << " << arg_name << ";\n";

          // store a object name to later generate header includes
          include_vector.push_back(arg_object);
        }
      }

      body << tab << tab << signal.get("name") << "(";

      // generate all arguments for the call to the virtual function
      unsigned int j = 0;
      for (auto ai = args.begin(); ai != args.end(); ++ai, ++j) {
        auto& arg = **ai;
        string arg_object;

        auto annotations_object = arg["annotation"].select(
            "name", "org.freedesktop.DBus.Object");
        if (!annotations_object.empty())
          arg_object = annotations_object.front()->get("value");

        auto arg_name = arg.get("name");
        if (arg_name.empty())
          arg_name = "arg" + to_string(j);

        if (!arg_object.empty())
          body << "_" << arg_name;
        else
          body << arg_name;

        if (ai + 1 != args.end())
          body << ", ";
      }

      body << ");\n" << tab << "}\n";
    }

    body << "};\n\n";

    for (unsigned int i = 0; i < nspaces; ++i)
      body << "} ";
    body << '\n';
  }

  body << "#endif //" << cond_comp;

  // remove all duplicates in the header include vector
  auto vec_end_it = unique(include_vector.begin(), include_vector.end());
  for (auto inc_it = include_vector.begin(); inc_it != vec_end_it; ++inc_it)
    head << "#include " << "\"" << *inc_it << ".h" << "\"\n";
  head << '\n';

  ofstream file(filename);
  if (file.bad()) {
    cerr << "unable to write file " << filename << endl;
    exit(-1);
  }

  file << head.str()
       << body.str();

  file.close();
}
