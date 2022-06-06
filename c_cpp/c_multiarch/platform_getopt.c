#include "platform/platform_getopt.h"

#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <string.h>


#define REQUIRE_ORDER   0
#define PERMUTE         1
#define RETURN_IN_ORDER 2


static inline void
exchange_a(char **argv, struct platform_getopt_ctx_a *d)
{
    int bottom = d->internal.first_nonopt;
    int middle = d->internal.last_nonopt;
    int top = d->internal.optind;
    char *tem;
    while (top > middle && middle > bottom)
    {
        if (top - middle > middle - bottom)
        {
            int len = middle - bottom;
            register int i;
            for (i = 0; i < len; i++)
            {
                tem = argv[bottom + i];
                argv[bottom + i] = argv[top - (middle - bottom) + i];
                argv[top - (middle - bottom) + i] = tem;
            }
            top -= len;
        }
        else
        {
            int len = top - middle;
            register int i;
            for (i = 0; i < len; i++)
            {
                tem = argv[bottom + i];
                argv[bottom + i] = argv[middle + i];
                argv[middle + i] = tem;
            }
            bottom += len;
        }
    }
    d->internal.first_nonopt += (d->internal.optind - d->internal.last_nonopt);
    d->internal.last_nonopt = d->internal.optind;
}


static inline const char *
platform_getopt_initialize_a(const char *optstring, struct platform_getopt_ctx_a *d,
                             int posixly_correct)
{
    d->internal.first_nonopt = d->internal.last_nonopt = d->internal.optind;
    d->internal.nextchar = NULL;
    d->internal.posixly_correct = posixly_correct | !!getenv("POSIXLY_CORRECT");
    if (optstring[0] == '-')
    {
        d->internal.ordering = RETURN_IN_ORDER;
        ++optstring;
    }
    else if (optstring[0] == '+')
    {
        d->internal.ordering = REQUIRE_ORDER;
        ++optstring;
    }
    else if (d->internal.posixly_correct)
        d->internal.ordering = REQUIRE_ORDER;
    else
        d->internal.ordering = PERMUTE;
    return optstring;
}


static inline int
platform_getopt_internal_r_a(int argc, char *const *argv, const char *optstring,
                             const struct platform_option_a *longopts, int *longind, int long_only,
                             struct platform_getopt_ctx_a *d, int posixly_correct)
{
    int print_errors = d->opterr;
    if (argc < 1)
        return -1;
    d->optarg = NULL;
    if (d->internal.optind == 0 || !d->internal.initialized)
    {
        if (d->internal.optind == 0)
            d->internal.optind = 1;
        optstring = platform_getopt_initialize_a(optstring, d, posixly_correct);
        d->internal.initialized = 1;
    }
    else if (optstring[0] == '-' || optstring[0] == '+')
        optstring++;
    if (optstring[0] == ':')
        print_errors = 0;
    if (d->internal.nextchar == NULL || *d->internal.nextchar == '\0')
    {
        if (d->internal.last_nonopt > d->internal.optind)
            d->internal.last_nonopt = d->internal.optind;
        if (d->internal.first_nonopt > d->internal.optind)
            d->internal.first_nonopt = d->internal.optind;
        if (d->internal.ordering == PERMUTE)
        {
            if (d->internal.first_nonopt != d->internal.last_nonopt
                && d->internal.last_nonopt != d->internal.optind)
                exchange_a((char **) argv, d);
            else if (d->internal.last_nonopt != d->internal.optind)
                d->internal.first_nonopt = d->internal.optind;
            while (d->internal.optind < argc && (argv[d->internal.optind][0] != '-' ||
                                                 argv[d->internal.optind][1] == '\0'))
                d->internal.optind++;
            d->internal.last_nonopt = d->internal.optind;
        }
        if (d->internal.optind != argc && !strcmp(argv[d->internal.optind], "--"))
        {
            d->internal.optind++;
            if (d->internal.first_nonopt != d->internal.last_nonopt
                && d->internal.last_nonopt != d->internal.optind)
                exchange_a((char **) argv, d);
            else if (d->internal.first_nonopt == d->internal.last_nonopt)
                d->internal.first_nonopt = d->internal.optind;
            d->internal.last_nonopt = argc;
            d->internal.optind = argc;
        }
        if (d->internal.optind == argc)
        {
            if (d->internal.first_nonopt != d->internal.last_nonopt)
                d->internal.optind = d->internal.first_nonopt;
            return -1;
        }
        if ((argv[d->internal.optind][0] != '-' || argv[d->internal.optind][1] == '\0'))
        {
            if (d->internal.ordering == REQUIRE_ORDER)
                return -1;
            d->optarg = argv[d->internal.optind++];
            return 1;
        }
        d->internal.nextchar = (argv[d->internal.optind] + 1
                               + (longopts != NULL && argv[d->internal.optind][1] == '-'));
    }
    if (longopts != NULL
        && (argv[d->internal.optind][1] == '-' ||
            (long_only && (argv[d->internal.optind][2]
                           || !strchr(optstring, argv[d->internal.optind][1])))))
    {
        char *nameend;
        unsigned int namelen;
        const struct platform_option_a *p;
        const struct platform_option_a *pfound = NULL;
        struct option_list
        {
            const struct platform_option_a *p;
            struct option_list *next;
        } *ambig_list = NULL;
        int exact = 0;
        int indfound = -1;
        int option_index;
        for (nameend = d->internal.nextchar; *nameend && *nameend != '='; nameend++)
            ;
        namelen = (unsigned int) (nameend - d->internal.nextchar);
        for (p = longopts, option_index = 0; p->name; p++, option_index++)
            if (!strncmp(p->name, d->internal.nextchar, namelen))
            {
                if (namelen == (unsigned int) strlen(p->name))
                {
                    pfound = p;
                    indfound = option_index;
                    exact = 1;
                    break;
                }
                else if (pfound == NULL)
                {
                    pfound = p;
                    indfound = option_index;
                }
                else if (long_only || pfound->has_arg != p->has_arg || pfound->val != p->val)
                {
                    struct option_list *newp = (struct option_list *) alloca(sizeof(*newp));
                    newp->p = p;
                    newp->next = ambig_list;
                    ambig_list = newp;
                }
            }
        if (ambig_list != NULL && !exact)
        {
            if (print_errors)
            {
                struct option_list first;
                first.p = pfound;
                first.next = ambig_list;
                ambig_list = &first;
                fprintf(stderr, "%s: option '%s' is ambiguous; possibilities:",
                        argv[0], argv[d->internal.optind]);
                do
                {
                    fprintf(stderr, " '--%s'", ambig_list->p->name);
                    ambig_list = ambig_list->next;
                } while (ambig_list != NULL);
                fputc('\n', stderr);
            }
            d->internal.nextchar += strlen(d->internal.nextchar);
            d->internal.optind++;
            d->optopt = 0;
            d->pfound = pfound;
            return '?';
        }
        if (pfound != NULL)
        {
            option_index = indfound;
            d->internal.optind++;
            if (*nameend)
            {
                if (pfound->has_arg)
                    d->optarg = nameend + 1;
                else
                {
                    if (print_errors)
                    {
                        if (argv[d->internal.optind - 1][1] == '-')
                        {
                            fprintf(stderr, "%s: option '--%s' doesn't allow an argument\n",
                                    argv[0], pfound->name);
                        }
                        else
                        {
                            fprintf(stderr, "%s: option '%c%s' doesn't allow an argument\n",
                                    argv[0],
                                    argv[d->internal.optind - 1][0],
                                    pfound->name);
                        }
                    }
                    d->internal.nextchar += strlen(d->internal.nextchar);
                    d->optopt = pfound->val;
                    d->pfound = pfound;
                    return '?';
                }
            }
            else if (pfound->has_arg == 1)
            {
                if (d->internal.optind < argc)
                    d->optarg = argv[d->internal.optind++];
                else
                {
                    if (print_errors)
                    {
                        fprintf(stderr,
                                "%s: option '--%s' requires an argument\n",
                                argv[0],
                                pfound->name);
                    }
                    d->internal.nextchar += strlen(d->internal.nextchar);
                    d->optopt = pfound->val;
                    d->pfound = pfound;
                    return optstring[0] == ':' ? ':' : '?';
                }
            }
            d->internal.nextchar += strlen(d->internal.nextchar);
            if (longind != NULL)
                *longind = option_index;
            d->pfound = pfound;
            return pfound->val;
        }
        if (!long_only || argv[d->internal.optind][1] == '-'
            || strchr(optstring, *d->internal.nextchar) == NULL)
        {
            if (print_errors)
            {
                if (argv[d->internal.optind][1] == '-')
                {
                    fprintf(stderr, "%s: unrecognized option '--%s'\n", argv[0],
                            d->internal.nextchar);
                }
                else
                {
                    fprintf(stderr, "%s: unrecognized option '%c%s'\n", argv[0],
                            argv[d->internal.optind][0],
                            d->internal.nextchar);
                }
            }
            d->internal.nextchar = (char *) "";
            d->internal.optind++;
            d->optopt = 0;
            d->pfound = pfound;
            return '?';
        }
    }
    {
        char c = *d->internal.nextchar++;
        char *temp = (char *) strchr(optstring, c);
        if (*d->internal.nextchar == '\0')
            ++d->internal.optind;
        if (temp == NULL || c == ':' || c == ';')
        {
            if (print_errors)
            {
                fprintf(stderr, "%s: invalid option -- '%c'\n", argv[0], c);
            }
            d->optopt = c;
            d->pfound = NULL;
            return '?';
        }
        if (temp[0] == 'W' && temp[1] == ';')
        {
            char *nameend;
            const struct platform_option_a *p;
            const struct platform_option_a *pfound = NULL;
            int exact = 0;
            int ambig = 0;
            int indfound = 0;
            int option_index;
            if (longopts == NULL)
                goto no_longs;
            if (*d->internal.nextchar != '\0')
            {
                d->optarg = d->internal.nextchar;
                d->internal.optind++;
            }
            else if (d->internal.optind == argc)
            {
                if (print_errors)
                {
                    fprintf(stderr, "%s: option requires an argument -- '%c'\n", argv[0], c);
                }
                d->optopt = c;
                if (optstring[0] == ':')
                    c = ':';
                else
                    c = '?';
                d->pfound = pfound;
                return c;
            }
            else
                d->optarg = argv[d->internal.optind++];
            for (d->internal.nextchar = nameend = d->optarg; *nameend && *nameend != '='; nameend++)
                ;
            for (p = longopts, option_index = 0; p->name; p++, option_index++)
                if (!strncmp(p->name, d->internal.nextchar, nameend - d->internal.nextchar))
                {
                    if ((unsigned int) (nameend - d->internal.nextchar) == strlen(p->name))
                    {
                        pfound = p;
                        indfound = option_index;
                        exact = 1;
                        break;
                    }
                    else if (pfound == NULL)
                    {
                        pfound = p;
                        indfound = option_index;
                    }
                    else if (long_only || pfound->has_arg != p->has_arg || pfound->val != p->val)
                        ambig = 1;
                }
            if (ambig && !exact)
            {
                if (print_errors)
                {
                    fprintf(stderr, "%s: option '-W %s' is ambiguous\n", argv[0], d->optarg);
                }
                d->internal.nextchar += strlen(d->internal.nextchar);
                d->internal.optind++;
                d->pfound = pfound;
                return '?';
            }
            if (pfound != NULL)
            {
                option_index = indfound;
                if (*nameend)
                {
                    if (pfound->has_arg)
                        d->optarg = nameend + 1;
                    else
                    {
                        if (print_errors)
                        {
                            fprintf(stderr, "%s: option '-W %s' doesn't allow an argument\n",
                                    argv[0], pfound->name);
                        }
                        d->internal.nextchar += strlen(d->internal.nextchar);
                        d->pfound = pfound;
                        return '?';
                    }
                }
                else if (pfound->has_arg == 1)
                {
                    if (d->internal.optind < argc)
                        d->optarg = argv[d->internal.optind++];
                    else
                    {
                        if (print_errors)
                        {
                            fprintf(stderr, "%s: option '-W %s' requires an argument\n",
                                    argv[0], pfound->name);
                        }
                        d->internal.nextchar += strlen(d->internal.nextchar);
                        d->pfound = pfound;
                        return optstring[0] == ':' ? ':' : '?';
                    }
                }
                else
                    d->optarg = NULL;
                d->internal.nextchar += strlen(d->internal.nextchar);
                if (longind != NULL)
                    *longind = option_index;
                d->pfound = pfound;
                return pfound->val;
            }
        no_longs:
            d->internal.nextchar = NULL;
            d->pfound = pfound;
            return 'W';
        }
        if (temp[1] == ':')
        {
            if (temp[2] == ':')
            {
                if (*d->internal.nextchar != '\0')
                {
                    d->optarg = d->internal.nextchar;
                    d->internal.optind++;
                }
                else
                    d->optarg = NULL;
                d->internal.nextchar = NULL;
            }
            else
            {
                if (*d->internal.nextchar != '\0')
                {
                    d->optarg = d->internal.nextchar;
                    d->internal.optind++;
                }
                else if (d->internal.optind == argc)
                {
                    if (print_errors)
                    {
                        fprintf(stderr, "%s: option requires an argument -- '%c'\n", argv[0], c);
                    }
                    d->optopt = c;
                    if (optstring[0] == ':')
                        c = ':';
                    else
                        c = '?';
                }
                else
                    d->optarg = argv[d->internal.optind++];
                d->internal.nextchar = NULL;
            }
        }
        d->pfound = NULL;
        return c;
    }
}


static inline int
platform_getopt_internal_a(int argc, char *const *argv, const char *optstring,
                           const struct platform_option_a *longopts, int *longind, int long_only,
                           int posixly_correct, struct platform_getopt_ctx_a *ctx)
{
    return platform_getopt_internal_r_a(argc, argv, optstring, longopts, longind, long_only, ctx,
                                        posixly_correct);
}


int
platform_getopt_a(int argc, char *const *argv, const char *optstring,
                  struct platform_getopt_ctx_a *ctx)
{
    return platform_getopt_internal_a(argc, argv, optstring, NULL, NULL, 0, 0, ctx);
}


int
platform_getopt_long_a(int argc, char *const *argv, const char *options,
                       const struct platform_option_a *long_options, int *opt_index,
                       struct platform_getopt_ctx_a *ctx)
{
    return platform_getopt_internal_a(argc, argv, options, long_options, opt_index, 0, 0, ctx);
}


int
platform_getopt_long_only_a(int argc, char *const *argv, const char *options,
                            const struct platform_option_a *long_options, int *opt_index,
                            struct platform_getopt_ctx_a *ctx)
{
    return platform_getopt_internal_a(argc, argv, options, long_options, opt_index, 1, 0, ctx);
}


static const char *
get_platform_argtype_str(enum platform_arg_type tp)
{
    switch (tp)
    {
        case platform_arg_type_none:
            return "   ";
        case platform_arg_type_string:
            return "STR";
        case platform_arg_type_integer:
            return "INT";
        case platform_arg_type_float:
            return "FLT";
    }
    return "   ";
}


void
platform_getopt_self_describe_option(struct platform_getopt_dynamic_arr *arg_cfg)
{
    unsigned int i;
    printf("usage:\n");
    for (i = 0; i < arg_cfg->optlen; ++i)
    {
        struct platform_option_a *arg = &arg_cfg->options[i];
        if (!arg->val)
            printf("       %s   ", get_platform_argtype_str(platform_arg_type_none));
        else
        {
            if (arg->has_arg == platform_arg_null_argument
                || arg->has_arg == platform_arg_no_argument)
                printf("    -%c %s   ", (char)(arg->val), get_platform_argtype_str(arg->arg_type));
            else if (arg->has_arg == platform_arg_required)
                printf("    -%c %s   ", (char)(arg->val), get_platform_argtype_str(arg->arg_type));
            else if (arg->has_arg == platform_arg_optional_argument)
                printf("    -%c [%s] ", (char)(arg->val), get_platform_argtype_str(arg->arg_type));
        }
        if (arg->name != NULL)
        {
            if (arg->has_arg == platform_arg_null_argument
                || arg->has_arg == platform_arg_no_argument)
            {
                assert(arg->arg_type == platform_arg_type_none);
                printf("--%-20s %s    ", arg->name, get_platform_argtype_str(arg->arg_type));
            }
            else if (arg->has_arg == platform_arg_required)
                printf("--%-20s=%s    ", arg->name, get_platform_argtype_str(arg->arg_type));
            else if (arg->has_arg == platform_arg_optional_argument)
                printf("--%-20s [=%s] ", arg->name, get_platform_argtype_str(arg->arg_type));
        }
        if (arg->description != NULL)
            printf("  %s;", arg->description);

        printf("\n");
    }
}
