#! /usr/bin/python
"""This script processes the XML generated by "make doc" and produces summary information
on symbols that libmiral intends to make public.

To use: Go to your build folder and run "make symbols"""""

from xml.dom import minidom
from sys import argv

HELPTEXT = __doc__
DEBUG = False

def _get_text(node):
    substrings = []
    for node in node.childNodes:
        if node.nodeType == node.TEXT_NODE:
            substrings.append(node.data)
        elif node.nodeType == node.ELEMENT_NODE:
            substrings.append(_get_text(node))
    return ''.join(substrings)

def _get_text_for_element(parent, tagname):
    substrings = []

    for node in parent.getElementsByTagName(tagname):
        substrings.append(_get_text(node))

    return ''.join(substrings)

def _get_file_location(node):
    for node in node.childNodes:
        if node.nodeType == node.ELEMENT_NODE and node.tagName == 'location':
            return node.attributes['file'].value
    if DEBUG:
        print 'no location in:', node
    return None

def _has_element(node, tagname):
    for node in node.childNodes:
        if node.nodeType == node.ELEMENT_NODE and node.tagName in tagname:
            return True
    return False

def _print_attribs(node, attribs):
    for attrib in attribs:
        print ' ', attrib, '=', node.attributes[attrib].value

def _concat_text_from_tags(parent, tagnames):
    substrings = []

    for tag in tagnames:
        substrings.append(_get_text_for_element(parent, tag))

    return ''.join(substrings)

def _print_location(node):
    print ' ', 'location', '=', _get_file_location(node)

def _get_attribs(node):
    kind = node.attributes['kind'].value
    static = node.attributes['static'].value
    prot = node.attributes['prot'].value
    return kind, static, prot

COMPONENT_MAP = {}
SYMBOLS = {'public' : set(), 'private' : set()}

def _report(publish, symbol):
    symbol = symbol.replace('~', '?')

    if publish:
        SYMBOLS['public'].add(symbol)
    else:
        SYMBOLS['private'].add(symbol)

    if not DEBUG:
        return

    if publish:
        print '  PUBLISH: {}'.format(symbol)
    else:
        print 'NOPUBLISH: {}'.format(symbol)

OLD_STANZAS = '''MIRAL_1.0 {
global:
  extern "C++" {
    miral::ActiveOutputsListener::?ActiveOutputsListener*;
    miral::ActiveOutputsListener::ActiveOutputsListener*;
    miral::ActiveOutputsListener::advise_output_begin*;
    miral::ActiveOutputsListener::advise_output_create*;
    miral::ActiveOutputsListener::advise_output_delete*;
    miral::ActiveOutputsListener::advise_output_end*;
    miral::ActiveOutputsListener::advise_output_update*;
    miral::ActiveOutputsListener::operator*;
    miral::ActiveOutputsMonitor::?ActiveOutputsMonitor*;
    miral::ActiveOutputsMonitor::ActiveOutputsMonitor*;
    miral::ActiveOutputsMonitor::add_listener*;
    miral::ActiveOutputsMonitor::delete_listener*;
    miral::ActiveOutputsMonitor::operator*;
    miral::ActiveOutputsMonitor::process_outputs*;
    miral::AddInitCallback::?AddInitCallback*;
    miral::AddInitCallback::AddInitCallback*;
    miral::AddInitCallback::operator*;
    miral::AppendEventFilter::AppendEventFilter*;
    miral::AppendEventFilter::operator*;
    miral::ApplicationAuthorizer::?ApplicationAuthorizer*;
    miral::ApplicationAuthorizer::ApplicationAuthorizer*;
    miral::ApplicationAuthorizer::operator*;
    miral::ApplicationCredentials::ApplicationCredentials*;
    miral::ApplicationCredentials::gid*;
    miral::ApplicationCredentials::pid*;
    miral::ApplicationCredentials::uid*;
    miral::ApplicationInfo::?ApplicationInfo*;
    miral::ApplicationInfo::ApplicationInfo*;
    miral::ApplicationInfo::add_window*;
    miral::ApplicationInfo::application*;
    miral::ApplicationInfo::name*;
    miral::ApplicationInfo::operator*;
    miral::ApplicationInfo::remove_window*;
    miral::ApplicationInfo::userdata*;
    miral::ApplicationInfo::windows*;
    miral::BasicSetApplicationAuthorizer::?BasicSetApplicationAuthorizer*;
    miral::BasicSetApplicationAuthorizer::BasicSetApplicationAuthorizer*;
    miral::BasicSetApplicationAuthorizer::operator*;
    miral::BasicSetApplicationAuthorizer::the_application_authorizer*;
    miral::CanonicalWindowManagerPolicy::CanonicalWindowManagerPolicy*;
    miral::CanonicalWindowManagerPolicy::advise_focus_gained*;
    miral::CanonicalWindowManagerPolicy::confirm_inherited_move*;
    miral::CanonicalWindowManagerPolicy::handle_modify_window*;
    miral::CanonicalWindowManagerPolicy::handle_raise_window*;
    miral::CanonicalWindowManagerPolicy::handle_window_ready*;
    miral::CanonicalWindowManagerPolicy::place_new_surface*;
    miral::CommandLineOption::?CommandLineOption*;
    miral::CommandLineOption::CommandLineOption*;
    miral::CommandLineOption::operator*;
    miral::CursorTheme::?CursorTheme*;
    miral::CursorTheme::CursorTheme*;
    miral::CursorTheme::operator*;
    miral::DebugExtension::DebugExtension*;
    miral::DebugExtension::disable*;
    miral::DebugExtension::enable*;
    miral::DebugExtension::operator*;
    miral::InternalClientLauncher::?InternalClientLauncher*;
    miral::InternalClientLauncher::InternalClientLauncher*;
    miral::InternalClientLauncher::launch*;
    miral::InternalClientLauncher::operator*;
    miral::Keymap::?Keymap*;
    miral::Keymap::Keymap*;
    miral::Keymap::operator*;
    miral::Keymap::set_keymap*;
    miral::MirRunner::?MirRunner*;
    miral::MirRunner::MirRunner*;
    miral::MirRunner::add_start_callback*;
    miral::MirRunner::add_stop_callback*;
    miral::MirRunner::run_with*;
    miral::MirRunner::set_exception_handler*;
    miral::MirRunner::stop*;
    miral::Output::?Output*;
    miral::Output::Output*;
    miral::Output::connected*;
    miral::Output::extents*;
    miral::Output::form_factor*;
    miral::Output::is_same_output*;
    miral::Output::operator*;
    miral::Output::orientation*;
    miral::Output::physical_size_mm*;
    miral::Output::pixel_format*;
    miral::Output::power_mode*;
    miral::Output::refresh_rate*;
    miral::Output::scale*;
    miral::Output::type*;
    miral::Output::used*;
    miral::Output::valid*;
    miral::SetCommandLineHandler::?SetCommandLineHandler*;
    miral::SetCommandLineHandler::SetCommandLineHandler*;
    miral::SetCommandLineHandler::operator*;
    miral::SetTerminator::?SetTerminator*;
    miral::SetTerminator::SetTerminator*;
    miral::SetTerminator::operator*;
    miral::SetWindowManagmentPolicy::?SetWindowManagmentPolicy*;
    miral::SetWindowManagmentPolicy::SetWindowManagmentPolicy*;
    miral::SetWindowManagmentPolicy::operator*;
    miral::StartupInternalClient::?StartupInternalClient*;
    miral::StartupInternalClient::StartupInternalClient*;
    miral::StartupInternalClient::operator*;
    miral::Window::?Window*;
    miral::Window::Window*;
    miral::Window::application*;
    miral::Window::move_to*;
    miral::Window::operator*;
    miral::Window::resize*;
    miral::Window::size*;
    miral::Window::top_left*;
    miral::WindowInfo::?WindowInfo*;
    miral::WindowInfo::WindowInfo*;
    miral::WindowInfo::add_child*;
    miral::WindowInfo::can_be_active*;
    miral::WindowInfo::children*;
    miral::WindowInfo::confine_pointer*;
    miral::WindowInfo::constrain_resize*;
    miral::WindowInfo::has_output_id*;
    miral::WindowInfo::height_inc*;
    miral::WindowInfo::is_visible*;
    miral::WindowInfo::max_aspect*;
    miral::WindowInfo::max_height*;
    miral::WindowInfo::max_width*;
    miral::WindowInfo::min_aspect*;
    miral::WindowInfo::min_height*;
    miral::WindowInfo::min_width*;
    miral::WindowInfo::must_have_parent*;
    miral::WindowInfo::must_not_have_parent*;
    miral::WindowInfo::name*;
    miral::WindowInfo::operator*;
    miral::WindowInfo::output_id*;
    miral::WindowInfo::parent*;
    miral::WindowInfo::preferred_orientation*;
    miral::WindowInfo::remove_child*;
    miral::WindowInfo::restore_rect*;
    miral::WindowInfo::userdata*;
    miral::WindowInfo::width_inc*;
    miral::WindowInfo::window*;
    miral::WindowManagementPolicy::?WindowManagementPolicy*;
    miral::WindowManagementPolicy::WindowManagementPolicy*;
    miral::WindowManagementPolicy::advise_begin*;
    miral::WindowManagementPolicy::advise_delete_app*;
    miral::WindowManagementPolicy::advise_delete_window*;
    miral::WindowManagementPolicy::advise_end*;
    miral::WindowManagementPolicy::advise_focus_gained*;
    miral::WindowManagementPolicy::advise_focus_lost*;
    miral::WindowManagementPolicy::advise_move_to*;
    miral::WindowManagementPolicy::advise_new_app*;
    miral::WindowManagementPolicy::advise_new_window*;
    miral::WindowManagementPolicy::advise_raise*;
    miral::WindowManagementPolicy::advise_resize*;
    miral::WindowManagementPolicy::operator*;
    miral::WindowManagerOptions::WindowManagerOptions*;
    miral::WindowManagerOptions::operator*;
    miral::WindowManagerTools::?WindowManagerTools*;
    miral::WindowManagerTools::WindowManagerTools*;
    miral::WindowManagerTools::active_display*;
    miral::WindowManagerTools::active_window*;
    miral::WindowManagerTools::ask_client_to_close*;
    miral::WindowManagerTools::count_applications*;
    miral::WindowManagerTools::drag_active_window*;
    miral::WindowManagerTools::find_application*;
    miral::WindowManagerTools::focus_next_application*;
    miral::WindowManagerTools::focus_next_within_application*;
    miral::WindowManagerTools::for_each_application*;
    miral::WindowManagerTools::force_close*;
    miral::WindowManagerTools::id_for_window*;
    miral::WindowManagerTools::info_for*;
    miral::WindowManagerTools::info_for_window_id*;
    miral::WindowManagerTools::invoke_under_lock*;
    miral::WindowManagerTools::modify_window*;
    miral::WindowManagerTools::operator*;
    miral::WindowManagerTools::place_and_size_for_state*;
    miral::WindowManagerTools::raise_tree*;
    miral::WindowManagerTools::select_active_window*;
    miral::WindowManagerTools::window_at*;
    miral::WindowSpecification::?WindowSpecification*;
    miral::WindowSpecification::WindowSpecification*;
    miral::WindowSpecification::aux_rect*;
    miral::WindowSpecification::aux_rect_placement_gravity*;
    miral::WindowSpecification::aux_rect_placement_offset*;
    miral::WindowSpecification::confine_pointer*;
    miral::WindowSpecification::height_inc*;
    miral::WindowSpecification::input_mode*;
    miral::WindowSpecification::input_shape*;
    miral::WindowSpecification::max_aspect*;
    miral::WindowSpecification::max_height*;
    miral::WindowSpecification::max_width*;
    miral::WindowSpecification::min_aspect*;
    miral::WindowSpecification::min_height*;
    miral::WindowSpecification::min_width*;
    miral::WindowSpecification::name*;
    miral::WindowSpecification::operator*;
    miral::WindowSpecification::output_id*;
    miral::WindowSpecification::parent*;
    miral::WindowSpecification::placement_hints*;
    miral::WindowSpecification::preferred_orientation*;
    miral::WindowSpecification::shell_chrome*;
    miral::WindowSpecification::size*;
    miral::WindowSpecification::state*;
    miral::WindowSpecification::top_left*;
    miral::WindowSpecification::type*;
    miral::WindowSpecification::update*;
    miral::WindowSpecification::userdata*;
    miral::WindowSpecification::width_inc*;
    miral::WindowSpecification::window_placement_gravity*;
    miral::apply_lifecycle_state_to*;
    miral::display_configuration_options*;
    miral::equivalent_display_area*;
    miral::kill*;
    miral::name_of*;
    miral::operator*;
    miral::pid_of*;
    miral::toolkit::Connection::Connection*;
    miral::toolkit::Surface::Surface*;
    non-virtual?thunk?to?miral::ActiveOutputsListener::?ActiveOutputsListener*;
    non-virtual?thunk?to?miral::ActiveOutputsListener::advise_output_begin*;
    non-virtual?thunk?to?miral::ActiveOutputsListener::advise_output_create*;
    non-virtual?thunk?to?miral::ActiveOutputsListener::advise_output_delete*;
    non-virtual?thunk?to?miral::ActiveOutputsListener::advise_output_end*;
    non-virtual?thunk?to?miral::ActiveOutputsListener::advise_output_update*;
    non-virtual?thunk?to?miral::ApplicationAuthorizer::?ApplicationAuthorizer*;
    non-virtual?thunk?to?miral::CanonicalWindowManagerPolicy::advise_focus_gained*;
    non-virtual?thunk?to?miral::CanonicalWindowManagerPolicy::confirm_inherited_move*;
    non-virtual?thunk?to?miral::CanonicalWindowManagerPolicy::handle_modify_window*;
    non-virtual?thunk?to?miral::CanonicalWindowManagerPolicy::handle_raise_window*;
    non-virtual?thunk?to?miral::CanonicalWindowManagerPolicy::handle_window_ready*;
    non-virtual?thunk?to?miral::CanonicalWindowManagerPolicy::place_new_surface*;
    non-virtual?thunk?to?miral::WindowManagementPolicy::?WindowManagementPolicy*;
    non-virtual?thunk?to?miral::WindowManagementPolicy::advise_begin*;
    non-virtual?thunk?to?miral::WindowManagementPolicy::advise_delete_app*;
    non-virtual?thunk?to?miral::WindowManagementPolicy::advise_delete_window*;
    non-virtual?thunk?to?miral::WindowManagementPolicy::advise_end*;
    non-virtual?thunk?to?miral::WindowManagementPolicy::advise_focus_gained*;
    non-virtual?thunk?to?miral::WindowManagementPolicy::advise_focus_lost*;
    non-virtual?thunk?to?miral::WindowManagementPolicy::advise_move_to*;
    non-virtual?thunk?to?miral::WindowManagementPolicy::advise_new_app*;
    non-virtual?thunk?to?miral::WindowManagementPolicy::advise_new_window*;
    non-virtual?thunk?to?miral::WindowManagementPolicy::advise_raise*;
    non-virtual?thunk?to?miral::WindowManagementPolicy::advise_resize*;
    non-virtual?thunk?to?miral::WindowManagementPolicy::advise_state_change*;
    typeinfo?for?miral::ActiveOutputsListener;
    typeinfo?for?miral::ActiveOutputsMonitor;
    typeinfo?for?miral::AddInitCallback;
    typeinfo?for?miral::AppendEventFilter;
    typeinfo?for?miral::ApplicationAuthorizer;
    typeinfo?for?miral::ApplicationCredentials;
    typeinfo?for?miral::ApplicationInfo;
    typeinfo?for?miral::BasicSetApplicationAuthorizer;
    typeinfo?for?miral::CanonicalWindowManagerPolicy;
    typeinfo?for?miral::CommandLineOption;
    typeinfo?for?miral::CursorTheme;
    typeinfo?for?miral::DebugExtension;
    typeinfo?for?miral::InternalClientLauncher;
    typeinfo?for?miral::Keymap;
    typeinfo?for?miral::MirRunner;
    typeinfo?for?miral::Output;
    typeinfo?for?miral::Output::PhysicalSizeMM;
    typeinfo?for?miral::SetCommandLineHandler;
    typeinfo?for?miral::SetTerminator;
    typeinfo?for?miral::SetWindowManagmentPolicy;
    typeinfo?for?miral::StartupInternalClient;
    typeinfo?for?miral::Window;
    typeinfo?for?miral::WindowInfo;
    typeinfo?for?miral::WindowManagementPolicy;
    typeinfo?for?miral::WindowManagerOption;
    typeinfo?for?miral::WindowManagerOptions;
    typeinfo?for?miral::WindowManagerTools;
    typeinfo?for?miral::WindowSpecification;
    typeinfo?for?miral::WindowSpecification::AspectRatio;
    typeinfo?for?miral::toolkit::Connection;
    typeinfo?for?miral::toolkit::PersistentId;
    typeinfo?for?miral::toolkit::Surface;
    typeinfo?for?miral::toolkit::SurfaceSpec;
    vtable?for?miral::ActiveOutputsListener;
    vtable?for?miral::ActiveOutputsMonitor;
    vtable?for?miral::AddInitCallback;
    vtable?for?miral::AppendEventFilter;
    vtable?for?miral::ApplicationAuthorizer;
    vtable?for?miral::ApplicationCredentials;
    vtable?for?miral::ApplicationInfo;
    vtable?for?miral::BasicSetApplicationAuthorizer;
    vtable?for?miral::CanonicalWindowManagerPolicy;
    vtable?for?miral::CommandLineOption;
    vtable?for?miral::CursorTheme;
    vtable?for?miral::DebugExtension;
    vtable?for?miral::InternalClientLauncher;
    vtable?for?miral::Keymap;
    vtable?for?miral::MirRunner;
    vtable?for?miral::Output;
    vtable?for?miral::Output::PhysicalSizeMM;
    vtable?for?miral::SetCommandLineHandler;
    vtable?for?miral::SetTerminator;
    vtable?for?miral::SetWindowManagmentPolicy;
    vtable?for?miral::StartupInternalClient;
    vtable?for?miral::Window;
    vtable?for?miral::WindowInfo;
    vtable?for?miral::WindowManagementPolicy;
    vtable?for?miral::WindowManagerOption;
    vtable?for?miral::WindowManagerOptions;
    vtable?for?miral::WindowManagerTools;
    vtable?for?miral::WindowSpecification;
    vtable?for?miral::WindowSpecification::AspectRatio;
    vtable?for?miral::toolkit::Connection;
    vtable?for?miral::toolkit::PersistentId;
    vtable?for?miral::toolkit::Surface;
    vtable?for?miral::toolkit::SurfaceSpec;
  };
#    miral::WindowInfo::can_morph_to*;
    _ZNK5miral10WindowInfo12can_morph_toE14MirSurfaceType;

#    miral::WindowInfo::needs_titlebar*;
    _ZN5miral10WindowInfo14needs_titlebarE14MirSurfaceType;

#    miral::WindowInfo::state*;
    _ZNK5miral10WindowInfo5stateEv;
    _ZN5miral10WindowInfo5stateE15MirSurfaceState;

#    miral::WindowInfo::type*;
    _ZN5miral10WindowInfo4typeE14MirSurfaceType;
    _ZNK5miral10WindowInfo4typeEv;

#    miral::WindowManagementPolicy::advise_state_change*;
    _ZN5miral22WindowManagementPolicy19advise_state_changeERKNS_10WindowInfoE15MirSurfaceState;
local: *;
};

MIRAL_1.1 {
global:
    # miral::WindowInfo::can_morph_to*
    _ZNK5miral10WindowInfo12can_morph_toE13MirWindowType;

    #miral::WindowInfo::needs_titlebar*;
    _ZN5miral10WindowInfo14needs_titlebarE13MirWindowType;

    # miral::WindowInfo::state*;
    _ZNK5miral10WindowInfo5stateEv;
    _ZN5miral10WindowInfo5stateE14MirWindowState;

    miral::WindowInfo::type*;
    _ZN5miral10WindowInfo4typeE13MirWindowType;
    _ZNK5miral10WindowInfo4typeEv;

    # miral::WindowManagementPolicy::advise_state_change*;
    _ZN5miral22WindowManagementPolicy19advise_state_changeERKNS_10WindowInfoE14MirWindowState;

  extern "C++" {'''

END_NEW_STANZA = '''  };
} MIRAL_1.0;'''

def _print_report():
    print OLD_STANZAS
    for symbol in sorted(SYMBOLS['public']):
        formatted_symbol = '    {};'.format(symbol)
        if formatted_symbol not in OLD_STANZAS and 'miral::toolkit::' not in formatted_symbol:
            print formatted_symbol
    print END_NEW_STANZA

def _print_debug_info(node, attributes):
    if not DEBUG:
        return
    print
    _print_attribs(node, attributes)
    _print_location(node)

def _parse_member_def(context_name, node, is_class):
    kind = node.attributes['kind'].value

    if (kind in ['enum', 'typedef']
        or _has_element(node, ['templateparamlist'])
        or kind in ['function'] and node.attributes['inline'].value == 'yes'):
        return

    name = _concat_text_from_tags(node, ['name'])

    if name in ['__attribute__']:
        if DEBUG:
            print '  ignoring doxygen mis-parsing:', _concat_text_from_tags(node, ['argsstring'])
        return

    if name.startswith('operator'):
        name = 'operator'

    if not context_name is None:
        symbol = context_name + '::' + name
    else:
        symbol = name

    is_function = kind == 'function'

    if is_function:
        _print_debug_info(node, ['kind', 'prot', 'static', 'virt'])
    else:
        _print_debug_info(node, ['kind', 'prot', 'static'])

    if DEBUG:
        print '  is_class:', is_class

    publish = _should_publish(is_class, is_function, node)

    _report(publish, symbol + '*')

    if is_function and node.attributes['virt'].value == 'virtual':
        _report(publish, 'non-virtual?thunk?to?' + symbol + '*')


def _should_publish(is_class, is_function, node):
    (kind, static, prot) = _get_attribs(node)

    publish = True

    if publish:
        publish = kind != 'define'

    if publish and is_class:
        publish = is_function or static == 'yes'

    if publish and prot == 'private':
        if is_function:
            publish = node.attributes['virt'].value == 'virtual'
        else:
            publish = False

    if publish and _has_element(node, ['argsstring']):
        publish = not _get_text_for_element(node, 'argsstring').endswith('=0')

    return publish


def _parse_compound_defs(xmldoc):
    compounddefs = xmldoc.getElementsByTagName('compounddef')
    for node in compounddefs:
        kind = node.attributes['kind'].value

        if kind in ['page', 'file', 'example', 'union']:
            continue

        if kind in ['group']:
            for member in node.getElementsByTagName('memberdef'):
                _parse_member_def(None, member, False)
            continue

        if kind in ['namespace']:
            symbol = _concat_text_from_tags(node, ['compoundname'])
            for member in node.getElementsByTagName('memberdef'):
                _parse_member_def(symbol, member, False)
            continue

        filename = _get_file_location(node)

        if DEBUG:
            print '  from file:', filename

        if ('/examples/' in filename or '/test/' in filename or '[generated]' in filename
            or '[STL]' in filename or _has_element(node, ['templateparamlist'])):
            continue

        symbol = _concat_text_from_tags(node, ['compoundname'])

        publish = True

        if publish:
            if kind in ['class', 'struct']:
                prot = node.attributes['prot'].value
                publish = prot != 'private'
                _print_debug_info(node, ['kind', 'prot'])
                _report(publish, 'vtable?for?' + symbol)
                _report(publish, 'typeinfo?for?' + symbol)

        if publish:
            for member in node.getElementsByTagName('memberdef'):
                _parse_member_def(symbol, member, kind in ['class', 'struct'])

if __name__ == "__main__":
    if len(argv) == 1 or '-h' in argv or '--help' in argv:
        print HELPTEXT
        exit()

    for arg in argv[1:]:
        try:
            if DEBUG:
                print 'Processing:', arg
            _parse_compound_defs(minidom.parse(arg))
        except Exception as error:
            print 'Error:', arg, error

    if DEBUG:
        print 'Processing complete'

    _print_report()
