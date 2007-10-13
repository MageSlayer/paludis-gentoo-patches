CLEANFILES = *~
MAINTAINERCLEANFILES = Makefile.in

noinst_LIBRARIES = libgtkpaludis.a
libgtkpaludis_a_SOURCES = \
	categories_list.cc                   categories_list.hh                   \
	categories_list_model.cc             categories_list_model.hh             \
	gui_task.cc                          gui_task.hh                          \
	gui_sync_task.cc                     gui_sync_task.hh                     \
	main_notebook.cc                     main_notebook.hh                     \
	main_notebook_page.cc                main_notebook_page.hh                \
	main_window.cc                       main_window.hh                       \
	markup.cc                            markup.hh                            \
	markup_formatter.cc                  markup_formatter.hh                  \
	messages_page.cc                     messages_page.hh                     \
	package_buttons.cc                   package_buttons.hh                   \
	packages_filter.cc                   packages_filter.hh                   \
	packages_list.cc                     packages_list.hh                     \
	packages_list_filtered_model.cc      packages_list_filtered_model.hh      \
	packages_list_model.cc               packages_list_model.hh               \
	packages_page.cc                     packages_page.hh                     \
	packages_package_filter.cc           packages_package_filter.hh           \
	packages_package_filter_model.cc     packages_package_filter_model.hh     \
	packages_repository_filter.cc        packages_repository_filter.hh        \
	packages_repository_filter_model.cc  packages_repository_filter_model.hh  \
	packages_text_filter.cc              packages_text_filter.hh              \
	packages_text_filter_source.cc       packages_text_filter_source.hh       \
	packages_text_filter_source_model.cc packages_text_filter_source_model.hh \
	query_notebook.cc                    query_notebook.hh                    \
	query_notebook_page.cc               query_notebook_page.hh               \
	query_window.cc                      query_window.hh                      \
	repositories_list.cc                 repositories_list.hh                 \
	repositories_list_model.cc           repositories_list_model.hh           \
	repositories_page.cc                 repositories_page.hh                 \
	repository_buttons.cc                repository_buttons.hh                \
	repository_info.cc                   repository_info.hh                   \
	repository_info_model.cc             repository_info_model.hh             \
	sets_list.cc                         sets_list.hh                         \
	sets_list_model.cc                   sets_list_model.hh                   \
	task_sequence_list.cc                task_sequence_list.hh                \
	task_sequence_list_model.cc          task_sequence_list_model.hh          \
	task_window.cc                       task_window.hh                       \
	threaded_window.cc                   threaded_window.hh                   \
	version_info.cc                      version_info.hh                      \
	version_info_model.cc                version_info_model.hh                \
	versions_list.cc                     versions_list.hh                     \
	versions_list_model.cc               versions_list_model.hh               \
	versions_page.cc                     versions_page.hh                     \
	packages_package_filter_option.hh                                         \
	packages_text_filter_source_option.hh

AM_CXXFLAGS = @PALUDIS_CXXFLAGS@ @PALUDIS_CXXFLAGS_VISIBILITY@ -I$(top_srcdir)/src/clients/gtkpaludis \
	      $(gtkmm_CFLAGS) $(gthread_CFLAGS) -I$(top_srcdir) \
	      @PALUDIS_CXXFLAGS_NO_WOLD_STYLE_CAST@ @PALUDIS_CXXFLAGS_NO_WREDUNDANT_DECLS@ @PALUDIS_CXXFLAGS_NO_WSHADOW@

built-sources : $(BUILT_SOURCES)
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s built-sources || exit 1 ; done

distcheck-deps : $(DISTCHECK_DEPS) distcheck-deps-subdirs

distcheck-deps-subdirs :
	for s in `echo $(SUBDIRS) | tr -d .` ; do $(MAKE) -C $$s distcheck-deps || exit 1 ; done

