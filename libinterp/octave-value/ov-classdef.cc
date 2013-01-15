/*

Copyright (C) 2012 Michael Goffioul

This file is part of Octave.

Octave is free software; you can redistribute it and/or modify it
under the terms of the GNU General Public License as published by the
Free Software Foundation; either version 3 of the License, or (at your
option) any later version.

Octave is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
for more details.

You should have received a copy of the GNU General Public License
along with Octave; see the file COPYING.  If not, see
<http://www.gnu.org/licenses/>.

*/

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <algorithm>
#include <map>

#include "defun.h"
#include "ov-builtin.h"
#include "ov-classdef.h"
#include "ov-fcn-handle.h"
#include "ov-typeinfo.h"
#include "pt-assign.h"
#include "pt-classdef.h"
#include "pt-funcall.h"
#include "pt-misc.h"
#include "pt-stmt.h"
#include "pt-walk.h"
#include "symtab.h"
#include "toplev.h"

#include "Array.cc"

static std::map<std::string, cdef_class> all_classes;
static std::map<std::string, cdef_package> all_packages;

static void
gripe_method_access (const std::string& from, const cdef_method& meth)
{
  octave_value acc = meth.get ("Access");
  std::string acc_s;

  if (acc.is_string ())
    acc_s = acc.string_value ();
  else
    acc_s = "class-restricted";

  error ("%s: method `%s' has %s access and cannot be run in this context",
	 from.c_str (), meth.get_name ().c_str (), acc_s.c_str ());
}

static void
gripe_property_access (const std::string& from, const cdef_property& prop,
		       bool is_set = false)
{
  octave_value acc = prop.get (is_set ? "SetAccess" : "GetAccess");
  std::string acc_s;

  if (acc.is_string ())
    acc_s = acc.string_value ();
  else
    acc_s = "class-restricted";

  if (is_set)
    error ("%s: property `%s' has %s access and cannot be set in this context",
	   from.c_str (), prop.get_name ().c_str (), acc_s.c_str ());
  else
    error ("%s: property `%s' has %s access and cannot be obtained in this context",
	   from.c_str (), prop.get_name ().c_str (), acc_s.c_str ());
}

static std::string
get_base_name (const std::string& nm)
{
  std::string::size_type pos = nm.find_last_of ('.');

  if (pos != std::string::npos)
    return nm.substr (pos + 1);

  return nm;
}

static void
make_function_of_class (const std::string& class_name,
                        const octave_value& fcn)
{
  octave_function *of = fcn.function_value ();

  if (! error_state)
    {
      of->stash_dispatch_class (class_name);

      octave_user_function *uf = of->user_function_value (true);

      if (! error_state && uf)
        {
          if (get_base_name (class_name) == uf->name ())
            {
              uf->mark_as_class_constructor ();
              uf->mark_as_classdef_constructor ();
            }
          else
            uf->mark_as_class_method ();
        }
    }
}

static void
make_function_of_class (const cdef_class& cls, const octave_value& fcn)
{
  make_function_of_class (cls.get_name (), fcn);
}

static octave_value
make_fcn_handle (octave_builtin::fcn ff, const std::string& nm)
{
  octave_value fcn (new octave_builtin (ff, nm));

  octave_value fcn_handle (new octave_fcn_handle (fcn, nm));

  return fcn_handle;
}

inline octave_value_list
execute_ov (octave_value val, const octave_value_list& args, int nargout)
{
  std::list<octave_value_list> idx (1, args);

  std::string type ("(");

  return val.subsref (type, idx, nargout);
}

static cdef_class
lookup_class (const std::string& name, bool error_if_not_found = true)
{
  std::map<std::string, cdef_class>::iterator it = all_classes.find (name);

  if (it == all_classes.end ())
    {
      // FIXME: implement this properly

      octave_value ov_cls = symbol_table::find (name);

      if (ov_cls.is_defined ())
        it = all_classes.find (name);
    }

  if (it == all_classes.end ())
    {
      if (error_if_not_found)
	error ("class not found: %s", name.c_str ());
    }
  else
    {
      cdef_class& cls = it->second;

      if (! cls.is_builtin ())
	{
	  // FIXME: check whether a class reload is needed
	}

      if (cls.ok ())
	return cls;
      else
	all_classes.erase (it);
    }

  return cdef_class ();
}

static cdef_class
lookup_class (const cdef_class& cls)
{
  // FIXME: placeholder for the time being, the purpose
  //        is to centralized any class update activity here.

  return cls;
}

static cdef_class
lookup_class (const octave_value& ov)
{
  if (ov.is_string())
    return lookup_class (ov.string_value ());
  else
    {
      cdef_class cls (to_cdef (ov));

      if (! error_state)
        return lookup_class (cls);
    }

  return cdef_class ();
}

static std::list<cdef_class>
lookup_classes (const Cell& cls_list)
{
  std::list<cdef_class> retval;

  for (int i = 0; i < cls_list.numel (); i++)
    {
      cdef_class c = lookup_class (cls_list(i));

      if (! error_state)
        retval.push_back (c);
      else
        {
          retval.clear ();
          break;
        }
    }

  return retval;
}

static octave_value
to_ov (const std::list<cdef_class>& class_list)
{
  Cell cls (class_list.size (), 1);
  int i = 0;

  for (std::list<cdef_class>::const_iterator it = class_list.begin ();
       it != class_list.end (); ++it, ++i)
    cls(i) = to_ov (*it);

  return octave_value (cls);
}

static bool
is_superclass (const cdef_class& clsa, const cdef_class& clsb,
	       bool allow_equal = true, int max_depth = -1)
{
  bool retval = false;

  if (allow_equal && clsa == clsb)
    retval = true;
  else if (max_depth != 0)
    {
      Cell c = clsb.get ("SuperClasses").cell_value ();

      for (int i = 0; ! error_state && ! retval && i < c.numel (); i++)
	{
	  cdef_class cls = lookup_class (c(i));

	  if (! error_state)
	    retval = is_superclass (clsa, cls, true,
                                    max_depth < 0 ? max_depth : max_depth-1);
	}
    }

  return retval;
}

inline bool
is_strict_superclass (const cdef_class& clsa, const cdef_class& clsb)
{ return is_superclass (clsa, clsb, false); }

inline bool
is_direct_superclass (const cdef_class& clsa, const cdef_class& clsb)
{ return is_superclass (clsa, clsb, false, 1); }

static octave_value_list
class_get_properties (const octave_value_list& args, int /* nargout */)
{
  octave_value_list retval;

  if (args.length () == 1 && args(0).type_name () == "object")
    {
      cdef_class cls (to_cdef (args(0)));

      retval(0) = cls.get_properties ();
    }

  return retval;
}

static cdef_class
get_class_context (std::string& name, bool& in_constructor)
{
  cdef_class cls;

  octave_function* fcn = octave_call_stack::current ();

  in_constructor = false;

  if (fcn &&
      (fcn->is_class_method ()
       || fcn->is_classdef_constructor ()
       || fcn->is_anonymous_function_of_class ()
       || (fcn->is_private_function ()
           && ! fcn->dispatch_class ().empty ())))
    {
      cls = lookup_class (fcn->dispatch_class ());
      if (! error_state)
        {
          name = fcn->name ();
          in_constructor = fcn->is_classdef_constructor ();
        }
    }

  return cls;
}

inline cdef_class
get_class_context (void)
{
  std::string dummy_string;
  bool dummy_bool;

  return get_class_context (dummy_string, dummy_bool);
}

static bool
check_access (const cdef_class& cls, const octave_value& acc)
{
  if (acc.is_string ())
    {
      std::string acc_s = acc.string_value ();

      if (acc_s == "public")
        return true;

      cdef_class ctx = get_class_context ();

      // The access is private or protected, this requires a
      // valid class context.

      if (! error_state && ctx.ok ())
        {
          if (acc_s == "private")
            return (ctx == cls);
          else if (acc_s == "protected")
            return is_superclass (cls, ctx);
          else
            panic_impossible ();
        }
    }
  else if (acc.is_cell ())
    {
      Cell acc_c = acc.cell_value ();

      cdef_class ctx = get_class_context ();

      // At this point, a class context is always required.

      if (! error_state && ctx.ok ())
        {
          if (ctx == cls)
            return true;

          for (int i = 0; ! error_state && i < acc.numel (); i++)
            {
              cdef_class acc_cls (to_cdef (acc_c(i)));

              if (! error_state)
                {
                  if (is_superclass (acc_cls, ctx))
                    return true;
                }
            }
        }
    }
  else
    error ("invalid property/method access in class `%s'",
           cls.get_name ().c_str ());
  
  return false;
}

static octave_value_list
class_get_methods (const octave_value_list& args, int /* nargout */)
{
  octave_value_list retval;

  if (args.length () == 1 && args(0).type_name () == "object")
    {
      cdef_class cls (to_cdef (args(0)));

      retval(0) = cls.get_methods ();
    }

  return retval;
}

static octave_value_list
class_get_superclasses (const octave_value_list& args, int /* nargout */)
{
  octave_value_list retval;

  if (args.length () == 1 && args(0).type_name () == "object"
      && args(0).class_name () == "meta.class")
    {
      cdef_class cls (to_cdef (args(0)));

      Cell classes = cls.get ("SuperClasses").cell_value ();

      retval(0) = to_ov (lookup_classes (classes));
    }

  return retval;
}

static octave_value_list
class_get_inferiorclasses (const octave_value_list& args, int /* nargout */)
{
  octave_value_list retval;

  if (args.length () == 1 && args(0).type_name () == "object"
      && args(0).class_name () == "meta.class")
    {
      cdef_class cls (to_cdef (args(0)));

      Cell classes = cls.get ("InferiorClasses").cell_value ();

      retval(0) = to_ov (lookup_classes (classes));
    }

  return retval;
}

static octave_value_list
class_fromName (const octave_value_list& args, int /* nargout */)
{
  octave_value_list retval;

  if (args.length () == 1)
    {
      std::string name = args(0).string_value ();

      if (! error_state)
	retval(0) = to_ov (lookup_class (name));
      else
	error ("fromName: invalid class name, expected a string value");
    }
  else
    error ("fromName: invalid number of parameters");

  return retval;
}

static octave_value_list
class_fevalStatic (const octave_value_list& args, int nargout)
{
  octave_value_list retval;

  if (args.length () > 1 && args(0).type_name () == "object")
    {
      cdef_class cls (to_cdef (args(0)));

      if (! error_state)
	{
	  std::string meth_name = args(1).string_value ();

	  if (! error_state)
	    {
	      cdef_method meth = cls.find_method (meth_name);

	      if (meth.ok ())
		{
		  if (meth.check_access ())
		    {
		      if (meth.is_static ())
			retval = meth.execute (args.splice (0, 2), nargout);
		      else
			error ("fevalStatic: method `%s' is not static",
			       meth_name.c_str ());
		    }
		  else
		    gripe_method_access ("fevalStatic", meth);
		}
	      else
		error ("fevalStatic: method not found: %s",
		       meth_name.c_str ());
	    }
	  else
	    error ("fevalStatic: invalid method name, expected a string value");
	}
      error ("fevalStatic: invalid object, expected a meta.class object");
    }
  else
    error ("fevalStatic: invalid arguments");

  return retval;
}

static octave_value_list
class_getConstant (const octave_value_list& args, int /* nargout */)
{
  octave_value_list retval;

  if (args.length () == 2 && args(0).type_name () == "object"
      && args(0).class_name () == "meta.class")
    {
      cdef_class cls = to_cdef (args(0));

      if (! error_state)
	{
	  std::string prop_name = args(1).string_value ();

	  if (! error_state)
	    {
	      cdef_property prop = cls.find_property (prop_name);

	      if (prop.ok ())
		{
		  if (prop.check_get_access ())
		    {
		      if (prop.is_constant ())
			retval(0) = prop.get_value ();
		      else
			error ("getConstant: property `%s' is not constant",
			       prop_name.c_str ());
		    }
		  else
		    gripe_property_access ("getConstant", prop);
		}
	      else
		error ("getConstant: property not found: %s",
		       prop_name.c_str ());
	    }
	  else
	    error ("getConstant: invalid property name, expected a string value");
	}
      else
	error ("getConstant: invalid object, expected a meta.class object");
    }
  else
    error ("getConstant: invalid arguments");

  return retval;
}

#define META_CLASS_CMP(OP, CLSA, CLSB, FUN) \
static octave_value_list \
class_ ## OP (const octave_value_list& args, int /* nargout */) \
{ \
  octave_value_list retval; \
\
  if (args.length () == 2 \
      && args(0).type_name () == "object" && args(1).type_name () == "object" \
      && args(0).class_name () == "meta.class" && args(1).class_name () == "meta.class") \
    { \
      cdef_class clsa = to_cdef (args(0)); \
\
      cdef_class clsb = to_cdef (args(1)); \
\
      if (! error_state) \
	retval(0) = FUN (CLSA, CLSB); \
      else \
	error (#OP ": invalid objects, expected meta.class objects"); \
    } \
  else \
    error (#OP ": invalid arguments"); \
\
  return retval; \
}

META_CLASS_CMP (lt, clsb, clsa, is_strict_superclass)
META_CLASS_CMP (le, clsb, clsa, is_superclass)
META_CLASS_CMP (gt, clsa, clsb, is_strict_superclass)
META_CLASS_CMP (ge, clsa, clsb, is_superclass)
META_CLASS_CMP (eq, clsa, clsb, operator==)
META_CLASS_CMP (ne, clsa, clsb, operator!=)

octave_value_list
property_get_defaultvalue (const octave_value_list& args, int /* nargout */)
{
  octave_value_list retval;

  if (args.length () == 1 && args(0).type_name () == "object")
    {
      cdef_property prop (to_cdef (args(0)));

      retval(0) = prop.get ("DefaultValue");

      if (! retval(0).is_defined ())
        error_with_id ("Octave:class:NotDefaultDefined",
                       "no default value for property `%s'",
                       prop.get_name ().c_str ());
    }

  return retval;
}

static octave_value_list
handle_delete (const octave_value_list& /* args */, int /* nargout */)
{
  octave_value_list retval;

  // FIXME: implement this

  return retval;
}

static cdef_class
make_class (const std::string& name,
            const std::list<cdef_class>& super_list = std::list<cdef_class> ())
{
  cdef_class cls (name, super_list);

  cls.set_class (cdef_class::meta_class ());
  cls.put ("Abstract", false);
  cls.put ("ConstructOnLoad", false);
  cls.put ("ContainingPackage", Matrix ());
  cls.put ("Description", std::string ());
  cls.put ("DetailedDescription", std::string ());
  cls.put ("Events", Cell ());
  cls.put ("Hidden", false);
  cls.put ("InferiorClasses", Cell ());
  cls.put ("Methods", Cell ());
  cls.put ("Properties", Cell ());
  cls.put ("Sealed", false);

  if (name == "handle")
    {
      cls.put ("HandleCompatible", true);
      cls.mark_as_handle_class ();
    }
  else if (super_list.empty ())
    {
      cls.put ("HandleCompatible", false);
    }
  else
    {
      bool all_handle_compatible = true;
      bool has_handle_class = false;

      for (std::list<cdef_class>::const_iterator it = super_list.begin ();
           it != super_list.end (); ++it)
        {
          all_handle_compatible = all_handle_compatible && it->get ("HandleCompatible").bool_value ();
          has_handle_class = has_handle_class || it->is_handle_class ();
        }

      if (has_handle_class && ! all_handle_compatible)
        ::error ("%s: cannot mix handle and non-HandleCompatible classes",
                 name.c_str ());
      else
        {
          cls.put ("HandleCompatible", all_handle_compatible);
          if (has_handle_class)
            cls.mark_as_handle_class ();
        }
    }

  if (error_state)
    return cdef_class ();

  if (! name.empty ())
    all_classes[name] = cls;

  return cls;
}

static cdef_class
make_class (const std::string& name, const cdef_class& super)
{
  return make_class (name, std::list<cdef_class> (1, super));
}

static cdef_class
make_meta_class (const std::string& name, const cdef_class& super)
{
  cdef_class cls = make_class (name, super);

  cls.put ("Sealed", true);
  cls.mark_as_meta_class ();

  return cls;
}

static cdef_property
make_property (const cdef_class& cls, const std::string& name,
	       const octave_value& get_method = Matrix (),
	       const std::string& get_access = "public",
	       const octave_value& set_method = Matrix (),
	       const std::string& set_access = "public")
{
  cdef_property prop (name);

  prop.set_class (cdef_class::meta_property ());
  prop.put ("Description", std::string ());
  prop.put ("DetailedDescription", std::string ());
  prop.put ("Abstract", false);
  prop.put ("Constant", false);
  prop.put ("GetAccess", get_access);
  prop.put ("SetAccess", set_access);
  prop.put ("Dependent", false);
  prop.put ("Transient", false);
  prop.put ("Hidden", false);
  prop.put ("GetObservable", false);
  prop.put ("SetObservable", false);
  prop.put ("GetMethod", get_method);
  prop.put ("SetMethod", set_method);
  prop.put ("DefiningClass", to_ov (cls));
  prop.put ("DefaultValue", octave_value ());
  prop.put ("HasDefault", false);

  std::string class_name = cls.get_name ();

  if (! get_method.is_empty ())
    make_function_of_class (class_name, get_method);
  if (! set_method.is_empty ())
    make_function_of_class (class_name, set_method);

  return prop;
}

inline cdef_property
make_attribute (const cdef_class& cls, const std::string& name)
{
  return make_property (cls, name, Matrix (), "public", Matrix (), "private");
}

static cdef_method
make_method (const cdef_class& cls, const std::string& name,
             const octave_value& fcn,const std::string& m_access = "public",
             bool is_static = false)
{
  cdef_method meth (name);

  meth.set_class (cdef_class::meta_method ());
  meth.put ("Abstract", false);
  meth.put ("Access", m_access);
  meth.put ("DefiningClass", to_ov (cls));
  meth.put ("Description", std::string ());
  meth.put ("DetailedDescription", std::string ());
  meth.put ("Hidden", false);
  meth.put ("Sealed", true);
  meth.put ("Static", is_static);

  if (fcn.is_defined ())
    make_function_of_class (cls, fcn);

  meth.set_function (fcn);

  return meth;
}

inline cdef_method
make_method (const cdef_class& cls, const std::string& name,
             octave_builtin::fcn ff, const std::string& m_access = "public",
             bool is_static = false)
{
  octave_value fcn (new octave_builtin (ff, name));

  return make_method (cls, name, fcn, m_access, is_static);
}

static cdef_package
make_package (const std::string& nm,
              const std::string& parent = std::string ())
{
  cdef_package pack ("meta.package");

  pack.set_class (cdef_class::meta_package ());
  pack.put ("Name", nm);
  pack.put ("ContainingPackage", to_ov (all_packages[parent]));

  if (! nm.empty ())
    all_packages[nm] = pack;

  return pack;
}

//----------------------------------------------------------------------------

DEFINE_OCTAVE_ALLOCATOR (octave_classdef);

int octave_classdef::t_id (-1);

const std::string octave_classdef::t_name ("object");

void
octave_classdef::register_type (void)
{
  t_id = octave_value_typeinfo::register_type
    (octave_classdef::t_name, "<unknown>", octave_value (new octave_classdef ()));
}

octave_value_list
octave_classdef::subsref (const std::string& type,
                          const std::list<octave_value_list>& idx,
                          int nargout)
{
  size_t skip = 0;
  octave_value_list retval;

  // FIXME: should check "subsref" method first

  retval = object.subsref (type, idx, nargout, skip, cdef_class ());

  if (! error_state)
    {
      if (type.length () > skip && idx.size () > skip)
	retval = retval(0).next_subsref (nargout, type, idx, skip);
    }

  return retval;
}

octave_value
octave_classdef::subsasgn (const std::string& type,
                           const std::list<octave_value_list>& idx,
                           const octave_value& rhs)
{
  return object.subsasgn (type, idx, rhs);
}

octave_value
octave_classdef::undef_subsasgn (const std::string& type,
                                 const std::list<octave_value_list>& idx,
                                 const octave_value& rhs)
{
  if (type.length () == 1 && type[0] == '(')
    {
      object = object.make_array ();

      if (! error_state)
        return subsasgn (type, idx, rhs);
    }
  else
    return octave_base_value::undef_subsasgn (type, idx, rhs);

  return octave_value ();
}

//----------------------------------------------------------------------------

class octave_classdef_proxy : public octave_function
{
public:
  octave_classdef_proxy (const cdef_class& _klass)
    : klass (_klass) { }

  ~octave_classdef_proxy (void)
    {
      // This means the class has been cleared from the symbol table.
      all_classes.erase (klass.get_name ());
    }

  octave_function* function_value (bool = false) { return this; }

  octave_value_list
  subsref (const std::string& type,
           const std::list<octave_value_list>& idx,
           int nargout)
    { return klass.subsref_meta (type, idx, nargout); }

  octave_value
  subsref (const std::string& type,
           const std::list<octave_value_list>& idx)
    {
      octave_value_list retval;

      retval = subsref (type, idx, 1);

      return (retval.length () > 0 ? retval(0) : octave_value ());
    }

  octave_value_list
  do_multi_index_op (int nargout, const octave_value_list& idx)
    {
      // Emulate constructor

      std::list<octave_value_list> l (1, idx);
      std::string type ("(");

      return subsref (type, l, nargout);
    }

  bool is_postfix_index_handled (char type) const
    { return (type == '(' || type == '.'); }

private:
  cdef_class klass;
};

//----------------------------------------------------------------------------

class octave_classdef_superclass_ref : public octave_function
{
public:
  octave_classdef_superclass_ref (const octave_value_list& a)
    : octave_function (), args (a) { }

  ~octave_classdef_superclass_ref (void) { }

  octave_value_list
  subsref (const std::string& type,
           const std::list<octave_value_list>& idx,
           int nargout)
    {
      size_t skip = 0;
      octave_value_list retval;

      switch (type[0])
        {
        case '(':
          skip = 1;
          retval = do_multi_index_op (type.length () > 1 ? 1 : nargout,
                                      idx.front ());
          break;
        default:
          retval = do_multi_index_op (1, octave_value_list ());
          break;
        }

      if (! error_state)
        {
          if (type.length () > skip && idx.size () > skip
              && retval.length () > 0)
            retval = retval(0).next_subsref (nargout, type, idx, skip);
        }

      return retval;
    }

  octave_value
  subsref (const std::string& type,
           const std::list<octave_value_list>& idx)
    {
      octave_value_list retval;

      retval = subsref (type, idx, 1);

      return (retval.length () > 0 ? retval(0) : octave_value ());
    }

  octave_value_list
  do_multi_index_op (int nargout, const octave_value_list& idx)
    {
      octave_value_list retval;

      std::string meth_name;
      bool in_constructor;
      cdef_class ctx;

      ctx = get_class_context (meth_name, in_constructor);

      if (! error_state && ctx.ok ())
        {
          std::string mname = args(0).string_value ();
          std::string pname = args(1).string_value ();
          std::string cname = args(2).string_value ();

          std::string cls_name = (pname.empty () ?
                                  cname : pname + "." + cname);
          cdef_class cls = lookup_class (cls_name);

          if (! error_state)
            {
              if (in_constructor)
                {
                  if (is_direct_superclass (cls, ctx))
                    {
                      if (is_constructed_object (mname))
                        {
                          octave_value& sym = symbol_table::varref (mname);

                          cls.run_constructor (to_cdef_ref (sym), idx);

                          retval(0) = sym;
                        }
                      else
                        ::error ("cannot call superclass constructor with "
                                 "variable `%s'", mname.c_str ());
                    }
                  else
                    ::error ("`%s' is not a direct superclass of `%s'",
                             cls_name.c_str (), ctx.get_name ().c_str ());
                }
              else
                {
                  if (mname == meth_name)
                    {
                      if (is_strict_superclass (cls, ctx))
                        {
                          // I see 2 possible implementations here:
                          // 1) use cdef_object::subsref with a different class
                          //    context; this avoids duplicating codem but
                          //    assumes the object is always the first argument
                          // 2) lookup the method manually and call
                          //    cdef_method::execute; this duplicates part of
                          //    logic in cdef_object::subsref, but avoid the
                          //    assumption of 1)
                          // Not being sure about the assumption of 1), I
                          // go with option 2) for the time being.

                          cdef_method meth = cls.find_method (meth_name, false);

                          if (meth.ok ())
                            {
                              if (meth.check_access ())
                                retval = meth.execute (idx, nargout);
                              else
                                gripe_method_access (meth_name, meth);
                            }
                          else
                            ::error ("no method `%s' found in superclass `%s'",
                                     meth_name.c_str (), cls_name.c_str ());
                        }
                      else
                        ::error ("`%s' is not a superclass of `%s'",
                                 cls_name.c_str (), ctx.get_name ().c_str ());
                    }
                  else
                    ::error ("method name mismatch (`%s' != `%s')",
                             mname.c_str (), meth_name.c_str ());
                }
            }
        }
      else if (! error_state)
        ::error ("superclass calls can only occur in methods or constructors");

      return retval;
    }

private:
  bool is_constructed_object (const std::string nm)
    {
      octave_function *of = octave_call_stack::current ();

      if (of->is_classdef_constructor ())
        {
          octave_user_function *uf = of->user_function_value (true);

          if (uf)
            {
              tree_parameter_list *ret_list = uf->return_list ();

              if (ret_list && ret_list->length () == 1)
                return (ret_list->front ()->name () == nm);
            }
        }

      return false;
    }

private:
  octave_value_list args;
};

//----------------------------------------------------------------------------

string_vector
cdef_object_rep::map_keys (void) const
{
  cdef_class cls = get_class ();

  if (cls.ok ())
    return cls.get_names ();
  
  return string_vector ();
}

octave_value_list
cdef_object_scalar::subsref (const std::string& type,
                             const std::list<octave_value_list>& idx,
                             int nargout, size_t& skip,
                             const cdef_class& context)
{
  skip = 0;

  cdef_class cls = (context.ok () ? context : get_class ());

  octave_value_list retval;

  if (! cls.ok ())
    return retval;

  switch (type[0])
    {
    case '.':
	{
	  std::string name = (idx.front ())(0).string_value ();

	  cdef_method meth = cls.find_method (name);

	  if (meth.ok ())
	    {
	      if (meth.check_access ())
		{
		  int _nargout = (type.length () > 2 ? 1 : nargout);

		  octave_value_list args;

		  skip = 1;

		  if (type.length () > 1 && type[1] == '(')
		    {
		      std::list<octave_value_list>::const_iterator it = idx.begin ();

		      args = *++it;

		      skip++;
		    }

		  if (meth.is_static ())
		    retval = meth.execute (args, _nargout);
		  else
		    {
		      refcount++;
		      retval = meth.execute (cdef_object (this), args, _nargout);
		    }
		}
	      else
		gripe_method_access ("subsref", meth);
	    }

	  if (skip == 0 && ! error_state)
	    {
	      cdef_property prop = cls.find_property (name);

	      if (prop.ok ())
		{
		  if (prop.check_get_access ())
		    {
		      refcount++;
		      retval(0) = prop.get_value (cdef_object (this));

		      skip = 1;
		    }
		  else
		    gripe_property_access ("subsref", prop);
		}
	      else
		error ("subsref: unknown method or property: %s", name.c_str ());
	    }
	  break;
	}
    default:
      error ("object cannot be indexed with `%c'", type[0]);
      break;
    }

  return retval;
}

octave_value
cdef_object_scalar::subsasgn (const std::string& type,
                              const std::list<octave_value_list>& idx,
                              const octave_value& rhs)
{
  octave_value retval;

  cdef_class cls = get_class ();

  switch (type[0])
    {
    case '.':
        {
          std::string name = (idx.front ())(0).string_value ();

          if (! error_state)
            {
              cdef_property prop = cls.find_property (name);

              if (prop.ok ())
                {
                  if (type.length () == 1)
                    {
                      if (prop.check_set_access ())
                        {
                          refcount++;

                          cdef_object obj (this);

                          prop.set_value (obj, rhs);

                          if (! error_state)
                            retval = to_ov (obj);
                        }
                      else
                        gripe_property_access ("subsasgn", prop, true);
                    }
                  else
                    {
                    }

                  if (! error_state)
                    {
                    }
                }
              else
                error ("subsasgn: unknown property: %s", name.c_str ());
            }
        }
      break;

    default:
      panic_impossible ();
      break;
    }

  return retval;
}

void
cdef_object_scalar::mark_for_construction (const cdef_class& cls)
{
  std::string cls_name = cls.get_name ();

  Cell supcls = cls.get ("SuperClasses").cell_value ();

  if (! error_state)
    {
      std::list<cdef_class> supcls_list = lookup_classes (supcls);

      if (! error_state)
        ctor_list[cls] = supcls_list;
    }
}

octave_value_list
cdef_object_array::subsref (const std::string& type,
                            const std::list<octave_value_list>& idx,
                            int /* nargout */, size_t& skip,
                            const cdef_class& /* context */)
{
  octave_value_list retval;

  skip = 1;

  switch (type[0])
    {
    case '(':
        {
          const octave_value_list& ival = idx.front ();
          bool is_scalar = true;
          Array<idx_vector> iv (dim_vector (1, ival.length ()));

          for (int i = 0; ! error_state && i < ival.length (); i++)
            {
              iv(i) = ival(i).index_vector ();
              if (! error_state)
                is_scalar = is_scalar && iv(i).is_scalar ();
            }

          if (! error_state)
            {
              Array<cdef_object> ires = array.index (iv);

              if (! error_state)
                {
                  if (is_scalar)
                    retval(0) = to_ov (ires(0));
                  else
                    {
                      cdef_object array_obj (new cdef_object_array (ires));

                      array_obj.set_class (get_class ());

                      retval(0) = to_ov (array_obj);
                    }
                }
            }
        }
      break;

    default:
      ::error ("can't perform indexing operation on array of %s objects",
               class_name ().c_str ());
      break;
    }

  return retval;
}

octave_value
cdef_object_array::subsasgn (const std::string& type,
                             const std::list<octave_value_list>& idx,
                             const octave_value& rhs)
{
  octave_value retval;

  switch (type[0])
    {
    case '(':
      if (type.length () == 1)
        {
          cdef_object rhs_obj = to_cdef (rhs);

          if (! error_state)
            {
              if (rhs_obj.get_class () == get_class ())
                {
                  const octave_value_list& ival = idx.front ();
                  bool is_scalar = true;
                  Array<idx_vector> iv (dim_vector (1, ival.length ()));

                  for (int i = 0; ! error_state && i < ival.length (); i++)
                    {
                      iv(i) = ival(i).index_vector ();
                      if (! error_state)
                        is_scalar = is_scalar && iv(i).is_scalar ();
                    }

                  if (! error_state)
                    {
                      Array<cdef_object> rhs_mat;

                      if (! rhs_obj.is_array ())
                        {
                          rhs_mat = Array<cdef_object> (dim_vector (1, 1));
                          rhs_mat(0) = rhs_obj;
                        }
                      else
                        rhs_mat = rhs_obj.array_value ();

                      if (! error_state)
                        {
                          octave_idx_type n = array.numel ();

                          array.assign (iv, rhs_mat, cdef_object ());

                          if (! error_state)
                            {
                              if (array.numel () > n)
                                fill_empty_values ();

                              if (! error_state)
                                {
                                  refcount++;
                                  retval = to_ov (cdef_object (this));
                                }
                            }
                        }
                    }
                }
              else
                ::error ("can't assign %s object into array of %s objects.",
                         rhs_obj.class_name ().c_str (),
                         class_name ().c_str ());
            }
        }
      else
        ::error ("can't perform indexing operation on array of %s objects",
                 class_name ().c_str ());
      break;

    default:
      ::error ("can't perform indexing operation on array of %s objects",
               class_name ().c_str ());
      break;
    }

  return retval;
}

void
cdef_object_array::fill_empty_values (void)
{
  cdef_class cls = get_class ();

  if (! error_state)
    {
      cdef_object obj;

      int n = array.numel ();

      for (int i = 0; ! error_state && i < n; i++)
        {
          if (! array.xelem (i).ok ())
            {
              if (! obj.ok ())
                {
                  obj = cls.construct_object (octave_value_list ());

                  if (! error_state)
                    array.xelem (i) = obj;
                }
              else
                array.xelem (i) = obj.copy ();
            }
        }
    }
}
  
bool cdef_object_scalar::is_constructed_for (const cdef_class& cls) const
{
  return (is_constructed ()
          || ctor_list.find (cls) == ctor_list.end ());
}

bool cdef_object_scalar::is_partially_constructed_for (const cdef_class& cls) const
{
  std::map< cdef_class, std::list<cdef_class> >::const_iterator it;

  if (is_constructed ())
    return true;
  else if ((it = ctor_list.find (cls)) == ctor_list.end ()
           || it->second.empty ())
    return true;

  for (std::list<cdef_class>::const_iterator lit = it->second.begin ();
       lit != it->second.end (); ++lit)
    if (! is_constructed_for (*lit))
      return false;

  return true;
}

handle_cdef_object::~handle_cdef_object (void)
{
  gnulib::printf ("deleting %s object (handle)\n",
                  get_class ().get_name ().c_str ());
}

value_cdef_object::~value_cdef_object (void)
{
  gnulib::printf ("deleting %s object (value)\n",
                  get_class ().get_name ().c_str ());
}

cdef_class::cdef_class_rep::cdef_class_rep (const std::list<cdef_class>& superclasses)
     : handle_cdef_object (), member_count (0), handle_class (false),
       object_count (0), meta (false)
{
  put ("SuperClasses", to_ov (superclasses));
  implicit_ctor_list = superclasses;
}

cdef_method
cdef_class::cdef_class_rep::find_method (const std::string& nm, bool local)
{
  method_iterator it = method_map.find (nm);

  if (it == method_map.end ())
    {
      // FIXME: look into class directory
    }
  else
    {
      cdef_method& meth = it->second;

      // FIXME: check if method reload needed

      if (meth.ok ())
	return meth;
    }

  if (! local)
    {
      // Look into superclasses

      Cell super_classes = get ("SuperClasses").cell_value ();

      for (int i = 0; i < super_classes.numel (); i++)
        {
          cdef_class cls = lookup_class (super_classes(i));

          if (! error_state)
            {
              cdef_method meth = cls.find_method (nm);

              if (meth.ok ())
                return meth;
            }
        }
    }

  return cdef_method ();
}

class ctor_analyzer : public tree_walker
{
public:
  ctor_analyzer (const std::string& ctor, const std::string& obj)
    : tree_walker (), who (ctor), obj_name (obj) { }

  void visit_statement_list (tree_statement_list& t)
    {
      for (tree_statement_list::const_iterator it = t.begin ();
           ! error_state && it != t.end (); ++it)
        (*it)->accept (*this);
    }

  void visit_statement (tree_statement& t)
    {
      if (t.is_expression ())
        t.expression ()->accept (*this);
    }

  void visit_simple_assignment (tree_simple_assignment& t)
    {
      t.right_hand_side ()->accept (*this);
    }

  void visit_multi_assignment (tree_multi_assignment& t)
    {
      t.right_hand_side ()->accept (*this);
    }

  void visit_index_expression (tree_index_expression& t)
    {
      t.expression ()->accept (*this);
    }

  void visit_funcall (tree_funcall& t)
    {
      octave_value fcn = t.function ();

      if (fcn.is_function ())
        {
          octave_function *of = fcn.function_value (true);

          if (of)
            {
              if (of->name () == "__superclass_reference__")
                {
                  octave_value_list args = t.arguments ();

                  if (args(0).string_value () == obj_name)
                    {
                      std::string package_name = args(1).string_value ();
                      std::string class_name = args(2).string_value ();

                      std::string ctor_name = (package_name.empty ()
                                               ? class_name
                                               : package_name + "." + class_name);

                      cdef_class cls = lookup_class (ctor_name, false);

                      if (cls.ok ())
                        ctor_list.push_back (cls);
                    }
                }
            }
        }
    }

  std::list<cdef_class> get_constructor_list (void) const
    { return ctor_list; }

  // NO-OP
  void visit_anon_fcn_handle (tree_anon_fcn_handle&) { }
  void visit_argument_list (tree_argument_list&) { }
  void visit_binary_expression (tree_binary_expression&) { }
  void visit_break_command (tree_break_command&) { }
  void visit_colon_expression (tree_colon_expression&) { }
  void visit_continue_command (tree_continue_command&) { }
  void visit_global_command (tree_global_command&) { }
  void visit_persistent_command (tree_persistent_command&) { }
  void visit_decl_elt (tree_decl_elt&) { }
  void visit_decl_init_list (tree_decl_init_list&) { }
  void visit_simple_for_command (tree_simple_for_command&) { }
  void visit_complex_for_command (tree_complex_for_command&) { }
  void visit_octave_user_script (octave_user_script&) { }
  void visit_octave_user_function (octave_user_function&) { }
  void visit_function_def (tree_function_def&) { }
  void visit_identifier (tree_identifier&) { }
  void visit_if_clause (tree_if_clause&) { }
  void visit_if_command (tree_if_command&) { }
  void visit_if_command_list (tree_if_command_list&) { }
  void visit_switch_case (tree_switch_case&) { }
  void visit_switch_case_list (tree_switch_case_list&) { }
  void visit_switch_command (tree_switch_command&) { }
  void visit_matrix (tree_matrix&) { }
  void visit_cell (tree_cell&) { }
  void visit_no_op_command (tree_no_op_command&) { }
  void visit_constant (tree_constant&) { }
  void visit_fcn_handle (tree_fcn_handle&) { }
  void visit_parameter_list (tree_parameter_list&) { }
  void visit_postfix_expression (tree_postfix_expression&) { }
  void visit_prefix_expression (tree_prefix_expression&) { }
  void visit_return_command (tree_return_command&) { }
  void visit_return_list (tree_return_list&) { }
  void visit_try_catch_command (tree_try_catch_command&) { }
  void visit_unwind_protect_command (tree_unwind_protect_command&) { }
  void visit_while_command (tree_while_command&) { }
  void visit_do_until_command (tree_do_until_command&) { }

private:
  /* The name of the constructor being analyzed */
  std::string who;

  /* The name of the first output argument of the constructor */
  std::string obj_name;

  /* The list of superclass constructors that are explicitly called */
  std::list<cdef_class> ctor_list;
};

void
cdef_class::cdef_class_rep::install_method (const cdef_method& meth)
{
  method_map[meth.get_name ()] = meth;

  member_count++;

  if (meth.is_constructor ())
    {
      // Analyze the constructor code to determine what superclass
      // constructors are called explicitly.

      octave_function *of = meth.get_function ().function_value (true);

      if (of)
        {
          octave_user_function *uf = of->user_function_value (true);

          if (uf)
            {
              tree_parameter_list *ret_list = uf->return_list ();
              tree_statement_list *body = uf->body ();

              if (ret_list && ret_list->size () == 1)
                {
                  std::string obj_name = ret_list->front ()->name ();
                  ctor_analyzer a (meth.get_name (), obj_name);

                  body->accept (a);
                  if (! error_state)
                    {
                      std::list<cdef_class> explicit_ctor_list
                        = a.get_constructor_list ();

                      for (std::list<cdef_class>::const_iterator it = explicit_ctor_list.begin ();
                           ! error_state && it != explicit_ctor_list.end (); ++it)
                        {
                          gnulib::printf ("explicit superclass constructor: %s\n",
                                          it->get_name ().c_str ());
                          implicit_ctor_list.remove (*it);
                        }
                    }
                }
              else
                ::error ("%s: invalid constructor output arguments",
                         meth.get_name ().c_str ());
            }
        }
    }
}

void
cdef_class::cdef_class_rep::load_all_methods (void)
{
  // FIXME: re-scan class directory
}

Cell
cdef_class::cdef_class_rep::get_methods (void)
{
  std::map<std::string,cdef_method> meths;

  find_methods (meths, false);

  if (! error_state)
    {
      Cell c (meths.size (), 1);

      int idx = 0;

      for (std::map<std::string,cdef_method>::const_iterator it = meths.begin ();
	   it != meths.end (); ++it, ++idx)
        c (idx, 0) = to_ov (it->second);

      return c;
    }

  return Cell ();
}

void
cdef_class::cdef_class_rep::find_methods (std::map<std::string, cdef_method>& meths,
                                          bool only_inherited)
{
  load_all_methods ();

  method_const_iterator it;

  for (it = method_map.begin (); it != method_map.end (); ++it)
    {
      if (! it->second.is_constructor ())
        {
          std::string nm = it->second.get_name ();

          if (meths.find (nm) == meths.end ())
            {
              if (only_inherited)
                {
                  octave_value acc = it->second.get ("Access");

                  if (! acc.is_string ()
                      || acc.string_value () == "private")
                    continue;
                }

              meths[nm] = it->second;
            }
        }
    }

  // Look into superclasses

  Cell super_classes = get ("SuperClasses").cell_value ();

  for (int i = 0; i < super_classes.numel (); i++)
    {
      cdef_class cls = lookup_class (super_classes(i));

      if (! error_state)
	cls.get_rep ()->find_methods (meths, true);
      else
	break;
    }
}

cdef_property
cdef_class::cdef_class_rep::find_property (const std::string& nm)
{
  property_iterator it = property_map.find (nm);

  if (it != property_map.end ())
    {
      cdef_property& prop = it->second;

      if (prop.ok ())
	return prop;
    }

  // Look into superclasses

  Cell super_classes = get ("SuperClasses").cell_value ();

  for (int i = 0; i < super_classes.numel (); i++)
    {
      cdef_class cls = lookup_class (super_classes(i));

      if (! error_state)
	{
	  cdef_property prop = cls.find_property (nm);

	  if (prop.ok ())
	    return prop;
	}
    }

  return cdef_property ();
}

void
cdef_class::cdef_class_rep::install_property (const cdef_property& prop)
{
  property_map[prop.get_name ()] = prop;

  member_count++;
}

Cell
cdef_class::cdef_class_rep::get_properties (void)
{
  std::map<std::string,cdef_property> props;

  find_properties (props, false);

  if (! error_state)
    {
      Cell c (props.size (), 1);

      int idx = 0;

      for (std::map<std::string,cdef_property>::const_iterator it = props.begin ();
	   it != props.end (); ++it, ++idx)
        c (idx, 0) = to_ov (it->second);

      return c;
    }

  return Cell ();
}

void
cdef_class::cdef_class_rep::find_properties (std::map<std::string,cdef_property>& props,
                                             bool only_inherited)
{
  property_const_iterator it;

  for (it = property_map.begin (); ! error_state && it != property_map.end ();
       ++it)
    {
      std::string nm = it->second.get_name ();

      if (props.find (nm) == props.end ())
	{
          if (only_inherited)
            {
              octave_value acc = it->second.get ("GetAccess");

              if (! acc.is_string ()
                  || acc.string_value () == "private")
                continue;
            }

	  props[nm] = it->second;
	}
    }

  // Look into superclasses

  Cell super_classes = get ("SuperClasses").cell_value ();

  for (int i = 0; ! error_state && i < super_classes.numel (); i++)
    {
      cdef_class cls = lookup_class (super_classes(i));

      if (! error_state)
	cls.get_rep ()->find_properties (props, true);
      else
	break;
    }
}

void
cdef_class::cdef_class_rep::find_names (std::set<std::string>& names,
                                        bool all)
{
  load_all_methods ();

  for (method_const_iterator it = method_map.begin ();
       ! error_state && it != method_map.end(); ++it)
    {
      if (! it->second.is_constructor ())
        {
          std::string nm = it->second.get_name ();

          if (! all)
            {
              octave_value acc = it->second.get ("Access");

              if (! acc.is_string()
                  || acc.string_value () != "public")
                continue;
            }

          names.insert (nm);
        }
    }

  for (property_const_iterator it = property_map.begin ();
       ! error_state && it != property_map.end (); ++it)
    {
      std::string nm = it->second.get_name ();

      if (! all)
        {
          octave_value acc = it->second.get ("GetAccess");

          if (! acc.is_string()
              || acc.string_value () != "public")
            continue;
        }

      names.insert (nm);
    }

  // Look into superclasses

  Cell super_classes = get ("SuperClasses").cell_value ();

  for (int i = 0; ! error_state && i < super_classes.numel (); i++)
    {
      cdef_class cls = lookup_class (super_classes(i));

      if (! error_state)
	cls.get_rep ()->find_names (names, all);
      else
	break;
    }
}

string_vector
cdef_class::cdef_class_rep::get_names (void)
{
  std::set<std::string> names;

  find_names (names, false);

  if (! error_state)
    {
      string_vector v (names.size ());

      int idx = 0;
      for (std::set<std::string>::const_iterator it = names.begin ();
	   it != names.end (); ++it, ++idx)
        v[idx] = *it;

      return v.sort (true);
    }

  return string_vector ();
}

void
cdef_class::cdef_class_rep::delete_object (cdef_object obj)
{
  method_iterator it = method_map.find ("delete");

  if (it != method_map.end ())
    {
      cdef_class cls = obj.get_class ();

      obj.set_class (wrap ());

      it->second.execute (obj, octave_value_list (), 0);

      obj.set_class (cls);
    }

  // FIXME: should we destroy corresponding properties here?

  // Call "delete" in super classes

  Cell super_classes = get ("SuperClasses").cell_value ();

  for (int i = 0; i < super_classes.numel (); i++)
    {
      cdef_class cls = lookup_class (super_classes(i));

      if (!error_state)
	cls.delete_object (obj);
    }
}

octave_value_list
cdef_class::cdef_class_rep::subsref_meta (const std::string& type,
                                          const std::list<octave_value_list>& idx,
                                          int nargout)
{
  octave_value_list retval;

  switch (type[0])
    {
    case '(':
      // Constructor call
      gnulib::printf ("constructor\n");
      retval(0) = construct (idx.front ());
      break;
    case '.':
      // Static method, constant (or property?)
      gnulib::printf ("static method\n");
      break;
    }

  if (! error_state)
    {
      if (type.length () > 1 && idx.size () > 1 && ! retval.empty ())
	retval = retval(0).next_subsref (nargout, type, idx);
    }

  return retval;
}

void
cdef_class::cdef_class_rep::initialize_object (cdef_object& obj)
{
  // Populate the object with default property values

  std::list<cdef_class> super_classes = lookup_classes (get ("SuperClasses").cell_value ());

  if (! error_state)
    {
      for (std::list<cdef_class>::iterator it = super_classes.begin ();
           ! error_state && it != super_classes.end (); ++it)
        it->initialize_object (obj);

      if (! error_state)
        {
          for (property_const_iterator it = property_map.begin ();
               ! error_state && it != property_map.end (); ++it)
            {
              if (! it->second.get ("Dependent").bool_value ())
                {
                  octave_value pvalue = it->second.get ("DefaultValue");

                  if (pvalue.is_defined ())
                    obj.put (it->first, pvalue);
                  else
                    obj.put (it->first, octave_value (Matrix ()));
                }
            }

          if (! error_state)
            {
              refcount++;
              obj.mark_for_construction (cdef_class (this));
            }
        }
    }
}

void
cdef_class::cdef_class_rep::run_constructor (cdef_object& obj,
                                             const octave_value_list& args)
{
  octave_value_list empty_args;

  for (std::list<cdef_class>::const_iterator it = implicit_ctor_list.begin ();
       ! error_state && it != implicit_ctor_list.end (); ++it)
    {
      cdef_class supcls = lookup_class (*it);

      if (! error_state)
        supcls.run_constructor (obj, empty_args);
    }

  if (error_state)
    return;

  std::string cls_name = get_name ();
  std::string ctor_name = get_base_name (cls_name);

  cdef_method ctor = find_method (ctor_name);

  if (ctor.ok ())
    {
      octave_value_list ctor_args (args);
      octave_value_list ctor_retval;

      ctor_args.prepend (to_ov (obj));
      ctor_retval = ctor.execute (ctor_args, 1);

      if (! error_state)
        {
          if (ctor_retval.length () == 1)
            obj = to_cdef (ctor_retval(0));
          else
            {
              ::error ("%s: invalid number of output arguments for classdef constructor",
                       ctor_name.c_str ());
              return;
            }
        }
    }

  obj.mark_as_constructed (wrap ());
}

octave_value
cdef_class::cdef_class_rep::construct (const octave_value_list& args)
{
  cdef_object obj = construct_object (args);

  if (! error_state && obj.ok ())
    return to_ov (obj);

  return octave_value ();
}

cdef_object
cdef_class::cdef_class_rep::construct_object (const octave_value_list& args)
{
  if (! is_abstract ())
    {
      cdef_object obj;

      if (is_meta_class ())
        {
          // This code path is only used to create empty meta objects
          // as filler for the empty values within a meta object array.

          cdef_class this_cls = wrap ();

          static cdef_object empty_class;

          if (this_cls == cdef_class::meta_class ())
            {
              if (! empty_class.ok ())
                empty_class = make_class ("", std::list<cdef_class> ());
              obj = empty_class;
            }
          else if (this_cls == cdef_class::meta_property ())
            {
              static cdef_property empty_property;

              if (! empty_class.ok ())
                empty_class = make_class ("", std::list<cdef_class> ());
              if (! empty_property.ok ())
                empty_property = make_property (empty_class, "");
              obj = empty_property;
            }
          else if (this_cls == cdef_class::meta_method ())
            {
              static cdef_method empty_method;

              if (! empty_class.ok ())
                empty_class = make_class ("", std::list<cdef_class> ());
              if (! empty_method.ok ())
                empty_method = make_method (empty_class, "", octave_value ());
              obj = empty_method;
            }
          else if (this_cls == cdef_class::meta_package ())
            {
              static cdef_package empty_package;

              if (! empty_package.ok ())
                empty_package = make_package ("");
              obj = empty_package;
            }
          else
            panic_impossible ();

          return obj;
        }
      else
        {
          if (is_handle_class ())
            obj = cdef_object (new handle_cdef_object ());
          else
            obj = cdef_object (new value_cdef_object ());
          obj.set_class (wrap ());

          initialize_object (obj);

          if (! error_state)
            {
              run_constructor (obj, args);

              if (! error_state)
                return obj;
            }
        }
    }
  else
    error ("cannot instantiate object for abstract class `%s'",
           get_name ().c_str ());

  return cdef_object ();
}

static octave_value
compute_attribute_value (tree_classdef_attribute* t)
{
  if (t->expression ())
    {
      if (t->expression ()->is_identifier ())
        {
          std::string s = t->expression ()->name ();

          if (s == "public")
            return std::string ("public");
          else if (s == "protected")
            return std::string ("protected");
          else if (s == "private")
            return std::string ("private");
        }

      return t->expression ()->rvalue1 ();
    }
  else
    return octave_value (true);
}

template<class T>
static std::string
attribute_value_to_string (T* t, octave_value v)
{
  if (v.is_string ())
    return v.string_value ();
  else if (t->expression ())
    return t->expression ()->original_text ();
  else
    return std::string ("true");
}

cdef_class
cdef_class::make_meta_class (tree_classdef* t)
{
  cdef_class retval;
  std::string class_name;

  // Class creation

  class_name = t->ident ()->name ();
  gnulib::printf ("class: %s\n", class_name.c_str ());

  std::list<cdef_class> slist;

  if (t->superclass_list ())
    {
      for (tree_classdef_superclass_list::iterator it = t->superclass_list ()->begin ();
           ! error_state && it != t->superclass_list ()->end (); ++it)
        {
          std::string sclass_name =
            ((*it)->package () ? (*it)->package ()->name () + "." : std::string ())
            + (*it)->ident ()->name ();

          gnulib::printf ("superclass: %s\n", sclass_name.c_str ());

          cdef_class sclass = lookup_class (sclass_name);

          if (! error_state)
            {
              if (! sclass.get ("Sealed").bool_value ())
                slist.push_back (sclass);
              else
                {
                  ::error ("`%s' cannot inherit from `%s', because it is sealed",
                           class_name.c_str (), sclass_name.c_str ());
                  return retval;
                }
            }
          else
            return retval;

        }
    }

  retval = ::make_class (class_name, slist);

  if (error_state)
    return cdef_class ();

  // Class attributes

  if (t->attribute_list ())
    {
      for (tree_classdef_attribute_list::iterator it = t->attribute_list ()->begin ();
           it != t->attribute_list ()->end (); ++it)
        {
          std::string aname = (*it)->ident ()->name ();
          octave_value avalue = compute_attribute_value (*it);

          gnulib::printf ("class attribute: %s = %s\n", aname.c_str (),
                  attribute_value_to_string (*it, avalue).c_str ());
          retval.put (aname, avalue);
        }
    }

  tree_classdef_body* b = t->body ();

  if (b)
    {
      // Method blocks

      std::list<tree_classdef_methods_block *> mb_list = b->methods_list ();

      for (tree_classdef_body::methods_list_iterator it = mb_list.begin ();
           it != mb_list.end (); ++it)
        {
          std::map<std::string, octave_value> amap;
          gnulib::printf ("method block\n");

          // Method attributes

          if ((*it)->attribute_list ())
            {
              for (tree_classdef_attribute_list::iterator ait = (*it)->attribute_list ()->begin ();
                   ait != (*it)->attribute_list ()->end (); ++ait)
                {
                  std::string aname = (*ait)->ident ()->name ();
                  octave_value avalue = compute_attribute_value (*ait);

                  gnulib::printf ("method attribute: %s = %s\n", aname.c_str (),
                          attribute_value_to_string (*ait, avalue).c_str ());
                  amap[aname] = avalue;
                }
            }

          // Methods

          if ((*it)->element_list ())
            {
              for (tree_classdef_methods_list::iterator mit = (*it)->element_list ()->begin ();
                   mit != (*it)->element_list ()->end (); ++mit)
                {
                  std::string mname = mit->function_value ()->name ();
                  cdef_method meth = make_method (retval, mname, *mit);

                  gnulib::printf ("%s: %s\n", (mname == class_name ? "constructor" : "method"),
                          mname.c_str ());
                  for (std::map<std::string, octave_value>::iterator ait = amap.begin ();
                       ait != amap.end (); ++ait)
                    meth.put (ait->first, ait->second);

                  retval.install_method (meth);
                }
            }
        }

      // Property blocks

      // FIXME: default property expression should be able to call static
      //        methods of the class being constructed. A restricted CLASSNAME
      //        symbol should be added to the scope before evaluating default
      //        value expressions.

      std::list<tree_classdef_properties_block *> pb_list = b->properties_list ();

      for (tree_classdef_body::properties_list_iterator it = pb_list.begin ();
           it != pb_list.end (); ++it)
        {
          std::map<std::string, octave_value> amap;
          gnulib::printf ("property block\n");

          // Property attributes

          if ((*it)->attribute_list ())
            {
              for (tree_classdef_attribute_list::iterator ait = (*it)->attribute_list ()->begin ();
                   ait != (*it)->attribute_list ()->end (); ++ait)
                {
                  std::string aname = (*ait)->ident ()->name ();
                  octave_value avalue = compute_attribute_value (*ait);

                  gnulib::printf ("property attribute: %s = %s\n", aname.c_str (),
                          attribute_value_to_string (*ait, avalue).c_str ());
                  if (aname == "Access")
                    {
                      amap["GetAccess"] = avalue;
                      amap["SetAccess"] = avalue;
                    }
                  else
                    amap[aname] = avalue;
                }
            }

          // Properties

          if ((*it)->element_list ())
            {
              for (tree_classdef_property_list::iterator pit = (*it)->element_list ()->begin ();
                   pit != (*it)->element_list ()->end (); ++pit)
                {
                  cdef_property prop = ::make_property (retval, (*pit)->ident ()->name ());

                  gnulib::printf ("property: %s\n", (*pit)->ident ()->name ().c_str ());
                  if ((*pit)->expression ())
                    {
                      octave_value pvalue = (*pit)->expression ()->rvalue1 ();

                      gnulib::printf ("property default: %s\n",
                              attribute_value_to_string (*pit, pvalue).c_str ());
                      prop.put ("DefaultValue", pvalue);
                    }

                  for (std::map<std::string, octave_value>::iterator ait = amap.begin ();
                       ait != amap.end (); ++ait)
                    prop.put (ait->first, ait->second);

                  retval.install_property (prop);
                }
            }
        }
    }

  return retval;
}

octave_function*
cdef_class::get_method_function (const std::string& /* nm */)
{
  octave_classdef_proxy* p = new octave_classdef_proxy (*this);

  return p;
}

octave_value
cdef_property::cdef_property_rep::get_value (const cdef_object& obj)
{
  octave_value retval;

  if (! obj.is_constructed ())
    {
      cdef_class cls (to_cdef (get ("DefiningClass")));

      if (! obj.is_partially_constructed_for (cls))
        {
          ::error ("cannot reference properties of class `%s' for non-constructed object",
                   cls.get_name ().c_str ());
          return retval;
        }
    }
 
  octave_value get_fcn = get ("GetMethod");

  // FIXME: should check whether we're already in get accessor method

  if (get_fcn.is_empty ())
    retval = obj.get (get ("Name").string_value ());
  else
    {
      octave_value_list args;

      args(0) = to_ov (obj);
      
      args = execute_ov (get_fcn, args, 1);

      if (! error_state)
	retval = args(0);
    }

  return retval;
}

bool
cdef_property::cdef_property_rep::is_recursive_set (const cdef_object& /* obj */) const
{
  // FIXME: implement
  return false;
}

void
cdef_property::cdef_property_rep::set_value (cdef_object& obj,
                                             const octave_value& val)
{
  if (! obj.is_constructed ())
    {
      cdef_class cls (to_cdef (get ("DefiningClass")));

      if (! obj.is_partially_constructed_for (cls))
        {
          ::error ("cannot reference properties of class `%s' for non-constructed object",
                   cls.get_name ().c_str ());
          return;
        }
    }
 
  octave_value set_fcn = get ("SetMethod");

  if (set_fcn.is_empty () || is_recursive_set (obj))
    {
      obj.put (get ("Name").string_value (), val);
    }
  else
    {
      octave_value_list args;

      args(0) = to_ov (obj);
      args(1) = val;

      args = execute_ov (set_fcn, args, 1);

      if (! error_state)
        {
          if (args.length() > 0)
            {
              cdef_object new_obj = to_cdef (args(0));

              if (! error_state)
                obj = new_obj;
            }
        }
    }
}

bool
cdef_property::check_get_access (void) const
{
  cdef_class cls (to_cdef (get ("DefiningClass")));

  if (! error_state)
    return ::check_access (cls, get ("GetAccess"));

  return false;
}

bool
cdef_property::check_set_access (void) const
{
  cdef_class cls (to_cdef (get ("DefiningClass")));

  if (! error_state)
    return ::check_access (cls, get ("SetAccess"));

  return false;
}

void
cdef_method::cdef_method_rep::check_method (void)
{
  // FIXME: check whether re-load is needed
}

octave_value_list
cdef_method::cdef_method_rep::execute (const octave_value_list& args,
				       int nargout)
{
  octave_value_list retval;

  if (! get ("Abstract").bool_value ())
    {
      check_method ();

      if (function.is_defined ())
	{
	  retval = execute_ov (function, args, nargout);
	}
    }
  else
    error ("%s: cannot execute abstract method",
	   get ("Name").string_value ().c_str ());

  return retval;
}

octave_value_list
cdef_method::cdef_method_rep::execute (const cdef_object& obj,
				       const octave_value_list& args,
				       int nargout)
{
  octave_value_list retval;

  if (! get ("Abstract").bool_value ())
    {
      check_method ();

      octave_value_list new_args;

      if (function.is_defined ())
	{
	  new_args.resize (args.length () + 1);

	  new_args(0) = to_ov (obj);
	  for (int i = 0; i < args.length (); i++)
	    new_args(i+1) = args(i);

	  retval = execute_ov (function, new_args, nargout);
	}
    }
  else
    error ("%s: cannot execute abstract method",
	   get ("Name").string_value ().c_str ());

  return retval;
}

bool
cdef_method::cdef_method_rep::is_constructor (void) const
{
  if (function.is_function())
    return function.function_value ()->is_classdef_constructor ();

  return false;
}

bool
cdef_method::check_access (void) const
{
  cdef_class cls (to_cdef (get ("DefiningClass")));

  if (! error_state)
    return ::check_access (cls, get ("Access"));

  return false;
}

static cdef_package
lookup_package (const std::string& name)
{
  std::map<std::string, cdef_package>::const_iterator it = all_packages.find (name);

  if (it != all_packages.end ())
    {
      cdef_package pack = it->second;

      if (pack.ok ())
        return pack;
      else
        error ("invalid package: %s", name.c_str ());
    }
  else
    error ("package not found: %s", name.c_str ());

  return cdef_package ();
}

static octave_value_list
package_fromName (const octave_value_list& args, int /* nargout */)
{
  octave_value_list retval;

  if (args.length () == 1)
    {
      std::string name = args(0).string_value ();

      if (! error_state)
	retval(0) = to_ov (lookup_package (name));
      else
	error ("fromName: invalid package name, expected a string value");
    }
  else
    error ("fromName: invalid number of parameters");

  return retval;
}

static octave_value_list
package_get_classes (const octave_value_list& args, int /* nargout */)
{
  octave_value_list retval (1, Matrix ());

  if (args.length () == 1 && args(0).type_name () == "object"
      && args(0).class_name () == "meta.package")
    {
      cdef_package pack (to_cdef (args(0)));

      retval(0) = pack.get_classes ();
    }

  return retval;
}

static octave_value_list
package_get_functions (const octave_value_list& args, int /* nargout */)
{
  octave_value_list retval (1, Matrix ());

  if (args.length () == 0 && args(0).type_name () == "object"
      && args(0).class_name () == "meta.package")
    {
      cdef_package pack (to_cdef (args(0)));

      retval(0) = pack.get_functions ();
    }

  return retval;
}

static octave_value_list
package_get_packages (const octave_value_list& args, int /* nargout */)
{
  octave_value_list retval (1, Matrix ());

  if (args.length () == 0 && args(0).type_name () == "object"
      && args(0).class_name () == "meta.package")
    {
      cdef_package pack (to_cdef (args(0)));

      retval(0) = pack.get_packages ();
    }

  return retval;
}

void
cdef_package::cdef_package_rep::install_class (const cdef_class& cls,
                                               const std::string& nm)
{
  class_map[nm] = cls;

  member_count++;
}

void
cdef_package::cdef_package_rep::install_function (const octave_value& fcn,
                                                  const std::string& nm)
{
  function_map[nm] = fcn;
}

void
cdef_package::cdef_package_rep::install_package (const cdef_package& pack,
                                                 const std::string& nm)
{
  package_map[nm] = pack;

  member_count++;
}

template<class T1, class T2>
Cell
map2Cell (const std::map<T1, T2>& m)
{
  Cell retval (1, m.size ());
  int i = 0;

  for (typename std::map<T1, T2>::const_iterator it = m.begin ();
       it != m.end (); ++it, ++i)
    {
      retval(i) = to_ov (it->second);
    }

  return retval;
}

Cell
cdef_package::cdef_package_rep::get_classes (void) const
{ return map2Cell (class_map); }

Cell
cdef_package::cdef_package_rep::get_functions (void) const
{ return map2Cell (function_map); }

Cell
cdef_package::cdef_package_rep::get_packages (void) const
{ return map2Cell (package_map); }

cdef_class cdef_class::_meta_class = cdef_class ();
cdef_class cdef_class::_meta_property = cdef_class ();
cdef_class cdef_class::_meta_method = cdef_class ();
cdef_class cdef_class::_meta_package = cdef_class ();

cdef_package cdef_package::_meta = cdef_package ();

void
install_classdef (void)
{
  octave_classdef::register_type ();

  /* bootstrap */
  cdef_class handle = make_class ("handle");
  cdef_class meta_class = cdef_class::_meta_class = make_meta_class ("meta.class", handle);
  handle.set_class (meta_class);
  meta_class.set_class (meta_class);

  /* meta classes */
  cdef_class meta_property = cdef_class::_meta_property = make_meta_class ("meta.property", handle);
  cdef_class meta_method = cdef_class::_meta_method = make_meta_class ("meta.method", handle);
  cdef_class meta_package = cdef_class::_meta_package = make_meta_class ("meta.package", handle);

  cdef_class meta_event = make_meta_class ("meta.event", handle);
  cdef_class meta_dynproperty = make_meta_class ("meta.dynamicproperty", handle);

  /* meta.class properties */
  meta_class.install_property (make_attribute (meta_class, "Abstract"));
  meta_class.install_property (make_attribute (meta_class, "ConstructOnLoad"));
  meta_class.install_property (make_property  (meta_class, "ContainingPackage"));
  meta_class.install_property (make_property  (meta_class, "Description"));
  meta_class.install_property (make_property  (meta_class, "DetailedDescription"));
  meta_class.install_property (make_property  (meta_class, "Events"));
  meta_class.install_property (make_attribute (meta_class, "HandleCompatible"));
  meta_class.install_property (make_attribute (meta_class, "Hidden"));
  meta_class.install_property
      (make_property (meta_class, "InferiorClasses",
		      make_fcn_handle (class_get_inferiorclasses, "meta.class>get.InferiorClasses"),
		      "public", Matrix (), "private"));
  meta_class.install_property
      (make_property  (meta_class, "Methods",
		       make_fcn_handle (class_get_methods, "meta.class>get.Methods"),
		       "public", Matrix (), "private"));
  meta_class.install_property
      (make_property  (meta_class, "MethodList",
		       make_fcn_handle (class_get_methods, "meta.class>get.MethodList"),
		       "public", Matrix (), "private"));
  meta_class.install_property (make_attribute (meta_class, "Name"));
  meta_class.install_property
      (make_property  (meta_class, "Properties",
		       make_fcn_handle (class_get_properties, "meta.class>get.Properties"),
		       "public", Matrix (), "private"));
  meta_class.install_property
      (make_property  (meta_class, "PropertyList",
		       make_fcn_handle (class_get_properties, "meta.class>get.PropertyList"),
		       "public", Matrix (), "private"));
  meta_class.install_property (make_attribute (meta_class, "Sealed"));
  meta_class.install_property
      (make_property (meta_class, "SuperClasses",
		      make_fcn_handle (class_get_superclasses, "meta.class>get.SuperClasses"),
		      "public", Matrix (), "private"));
  meta_class.install_property
      (make_property (meta_class, "SuperClassList",
		      make_fcn_handle (class_get_superclasses, "meta.class>get.SuperClassList"),
		      "public", Matrix (), "private"));
  /* meta.class methods */
  meta_class.install_method (make_method (meta_class, "fromName", class_fromName,
					  "public", true));
  meta_class.install_method (make_method (meta_class, "fevalStatic", class_fevalStatic,
					  "public", false));
  meta_class.install_method (make_method (meta_class, "getConstant", class_getConstant,
					  "public", false));
  meta_class.install_method (make_method (meta_class, "eq", class_eq));
  meta_class.install_method (make_method (meta_class, "ne", class_ne));
  meta_class.install_method (make_method (meta_class, "lt", class_lt));
  meta_class.install_method (make_method (meta_class, "le", class_le));
  meta_class.install_method (make_method (meta_class, "gt", class_gt));
  meta_class.install_method (make_method (meta_class, "ge", class_ge));

  /* meta.method properties */
  meta_method.install_property (make_attribute (meta_method, "Abstract"));
  meta_method.install_property (make_attribute (meta_method, "Access"));
  meta_method.install_property (make_attribute (meta_method, "DefiningClass"));
  meta_method.install_property (make_attribute (meta_method, "Description"));
  meta_method.install_property (make_attribute (meta_method, "DetailedDescription"));
  meta_method.install_property (make_attribute (meta_method, "Hidden"));
  meta_method.install_property (make_attribute (meta_method, "Name"));
  meta_method.install_property (make_attribute (meta_method, "Sealed"));
  meta_method.install_property (make_attribute (meta_method, "Static"));

  /* meta.property properties */
  meta_property.install_property (make_attribute (meta_property, "Name"));
  meta_property.install_property (make_attribute (meta_property, "Description"));
  meta_property.install_property (make_attribute (meta_property, "DetailedDescription"));
  meta_property.install_property (make_attribute (meta_property, "Abstract"));
  meta_property.install_property (make_attribute (meta_property, "Constant"));
  meta_property.install_property (make_attribute (meta_property, "GetAccess"));
  meta_property.install_property (make_attribute (meta_property, "SetAccess"));
  meta_property.install_property (make_attribute (meta_property, "Dependent"));
  meta_property.install_property (make_attribute (meta_property, "Transient"));
  meta_property.install_property (make_attribute (meta_property, "Hidden"));
  meta_property.install_property (make_attribute (meta_property, "GetObservable"));
  meta_property.install_property (make_attribute (meta_property, "SetObservable"));
  meta_property.install_property (make_attribute (meta_property, "GetMethod"));
  meta_property.install_property (make_attribute (meta_property, "SetMethod"));
  meta_property.install_property (make_attribute (meta_property, "DefiningClass"));
  meta_property.install_property
      (make_property (meta_property, "DefaultValue",
		      make_fcn_handle (property_get_defaultvalue, "meta.property>get.DefaultValue"),
		      "public", Matrix (), "private"));
  meta_property.install_property (make_attribute (meta_property, "HasDefault"));
  /* meta.property events */
  // FIXME: add events

  /* handle methods */
  handle.install_method (make_method (handle, "delete", handle_delete));

  /* meta.package properties */
  meta_package.install_property (make_attribute (meta_package, "Name"));
  meta_package.install_property (make_property  (meta_package, "ContainingPackage"));
  meta_package.install_property
      (make_property (meta_package, "ClassList",
		      make_fcn_handle (package_get_classes, "meta.package>get.ClassList"),
		      "public", Matrix (), "private"));
  meta_package.install_property
      (make_property (meta_package, "Classes",
		      make_fcn_handle (package_get_classes, "meta.package>get.Classes"),
		      "public", Matrix (), "private"));
  meta_package.install_property
      (make_property (meta_package, "FunctionList",
		      make_fcn_handle (package_get_functions, "meta.package>get.FunctionList"),
		      "public", Matrix (), "private"));
  meta_package.install_property
      (make_property (meta_package, "Functions",
		      make_fcn_handle (package_get_functions, "meta.package>get.Functions"),
		      "public", Matrix (), "private"));
  meta_package.install_property
      (make_property (meta_package, "PackageList",
		      make_fcn_handle (package_get_packages, "meta.package>get.PackageList"),
		      "public", Matrix (), "private"));
  meta_package.install_property
      (make_property (meta_package, "Packages",
		      make_fcn_handle (package_get_packages, "meta.package>get.Packages"),
		      "public", Matrix (), "private"));
  meta_package.install_method (make_method (meta_package, "fromName", package_fromName,
                                            "public", true));

  /* create "meta" package */
  cdef_package package_meta = cdef_package::_meta = make_package ("meta");
  package_meta.install_class (meta_class,       "class");
  package_meta.install_class (meta_property,    "property");
  package_meta.install_class (meta_method,      "method");
  package_meta.install_class (meta_package,     "package");
  package_meta.install_class (meta_event,       "event");
  package_meta.install_class (meta_dynproperty, "dynproperty");
}

DEFUN (__meta_get_package__, args, , "")
{
  octave_value retval;

  if (args.length () == 1)
    {
      std::string cname = args(0).string_value ();

      if (! error_state)
	retval = to_ov (lookup_package (cname));
      else
	error ("invalid package name, expected a string value");
    }
  else
    print_usage ();

  return retval;
}

DEFUN (__superclass_reference__, args, /* nargout */,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} __superclass_reference__ ()\n\
Undocumented internal function.\n\
@end deftypefn")
{
  return octave_value (new octave_classdef_superclass_ref (args));
}

DEFUN (__meta_class_query__, args, /* nargout */,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} __meta_class_query__ ()\n\
Undocumented internal function.\n\
@end deftypefn")
{
  octave_value retval;

  std::cerr << "__meta_class_query__ ("
            << args(0).string_value () << ", "
            << args(1).string_value () << ")"
            << std::endl;

  if (args.length () == 2)
    {
      std::string pkg = args(0).string_value ();
      std::string cls = args(1).string_value ();

      if (! pkg.empty ())
        cls = pkg + "." + cls;

      if (! error_state)
	retval = to_ov (lookup_class (cls));
      else
	error ("invalid class name, expected a string value");
    }
  else
    print_usage ();

  return retval;
}

DEFUN (metaclass, args, /* nargout */,
  "-*- texinfo -*-\n\
@deftypefn {Built-in Function} {} metaclass (obj)\n\
Returns the meta.class object corresponding to the class of @var{obj}.\n\
@end deftypefn")
{
  octave_value retval;

  if (args.length () == 1)
    {
      cdef_object obj = to_cdef (args(0));

      if (! error_state)
        retval = to_ov (obj.get_class ());
      else
        print_usage ();
    }
  else
    print_usage ();

  return retval;
}

/*
;;; Local Variables: ***
;;; mode: C++ ***
;;; End: ***
*/
