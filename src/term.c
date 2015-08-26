#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "term.h"
#include "utils.h"
#include "environment.h"

/*
    if (t->kind == ANN){
    }else if (t->kind == VAR){
    }else if (t->kind == APP){
    }else if (t->kind == FUN){
    }else if (t->kind == IND){
    }else if (t->kind == PI){
    }else if (t->kind == TYPE){
    }
*/

int compare_types(term *t1, term *t2)
{
    if(DEBUG) printf("Comparing ");
    if(DEBUG) print_term(t1);
    if(DEBUG) printf(" and ");
    if(DEBUG) print_term(t2);
    if(DEBUG) printf("\n");

    if(!t1 && !t2) return 1;
    if(!t1 || !t2) return 0;
    if(t1->kind != t2->kind) return 0;
    if(t1->kind == VAR) return t1->n == t2->n;
    if(t1->kind == TYPE && t2->kind == TYPE) return 1;
    if(t1->kind == IND && t2->kind == IND) return (strcmp(t1->name, t2->name) == 0);
    return (compare_types(t1->left, t2->left) && compare_types(t1->right, t2->right));
}

void free_term_contents(term *t)
{
    if(!t) return;
    free_term(t->left);
    free_term(t->right);
    free_term(t->annotation);
    if (t->name) free(t->name);
    if (t->cases){
        int i;
        for(i = 0; i < t->n; ++i){
            free_term(t->cases[i]);
        }
        free(t->cases);
    }
}

void free_term(term *t)
{
    if(!t) return;
    free_term_contents(t);
    free(t);
}

term *copy_term(term *t)
{
    if(!t) return 0;
    term *copy = calloc(1, sizeof(term));
    copy->kind = t->kind;
    copy->left  = copy_term(t->left);
    copy->right = copy_term(t->right);
    copy->annotation = copy_term(t->annotation);
    if (t->name) copy->name = copy_string(t->name);
    copy->n = t->n;
    /*
    if (t->kind == IND){
        not_implemented();
    }
    */
    if(t->cases){
        copy->cases = calloc(t->n, sizeof(term*));
        int i;
        for(i = 0; i < t->n; ++i){
            copy->cases[i] = copy_term(t->cases[i]);
        }
    }
    return copy;
}

void increment(term *t, int n, int level)
{
    if (!t) return;
    if (t->annotation) increment(t->annotation, n, level);
    if (t->kind == VAR){
        if (t->n >= level) t->n += n;
    }else if (t->kind == APP){
        increment(t->left,  n, level);
        increment(t->right, n, level);
    }else if (t->kind == FUN || t->kind == PI){
        increment(t->left->annotation, n, level);
        increment(t->right, n, level+1);
    }else if (t->kind == IND){
    }else if (t->kind == TYPE){
    }
}

void substitute(term *t, term *arg, int level)
{
    if(!t) return;
    if (t->kind == VAR){
        if(t->n == level){
            term *copy = copy_term(arg);       
            increment(copy, level, 0);
            free_term_contents(t);
            *t = *copy;
            free(copy);
        }else if(t->n > level){
            t->n -= 1;
        }
    }else if (t->kind == APP){
        substitute(t->left, arg, level);
        substitute(t->right, arg, level);
    }else if (t->kind == IND){
    }else if (t->kind == PI || t->kind == FUN){
        substitute(t->left->annotation, arg, level);
        substitute(t->right, arg, level+1);
    }else if (t->kind == TYPE){
    }
    else if(t->kind == ELIM){
        int i;
        substitute(t->left, arg, level);
        substitute(t->right, arg, level);
        for(i = 0; i < t->n; ++i){
            substitute(t->cases[i], arg, level);
        }
    }
}

void replace_term(term *old, term *new)
{
    term *copy = copy_term(new);
    free_term_contents(old);
    *old = *copy;
    free(copy);
}

term *resolve(term *t, environment *env, term_list *context)
{
    if(DEBUG) printf("Resolving: ");
    if(DEBUG) print_term(t);
    if(DEBUG) printf("\n");
    
    term *l = 0;
    if (t->name) {
        l = get_environment(env, t->name);
        if(l){
            if(l->kind == DEF){
                l = l->right;
            }
        }
    }
    else l = get_term_list(context, t->n);
    if (!l){
        if(DEBUG) printf("VAR not found in env or context\n");
        return 0;
    }
    if(DEBUG) printf("Found: ");
    if(DEBUG) print_term(l->annotation);
    if(DEBUG) printf("\n");
    return l->annotation;
}

term *make_type()
{
    term *t = calloc(1, sizeof(term));
    t->kind = TYPE;
    return t;
}

term *make_variable(char *s)
{
    term *var = calloc(1, sizeof(term));
    var->name = s;
    var->kind = VAR;
    return var;
}

term *make_pi(term *l, term *r)
{
    term *pi = calloc(1, sizeof(term));
    pi->kind = PI;
    pi->left = l;
    pi->right = r;
    return pi;
}

term *make_app(term *l, term *r)
{
    term *app = calloc(1, sizeof(term));
    app->kind = APP;
    app->left = l;
    app->right = r;
    return app;
}

term *construct_instance(term *cons)
{
    term *annot = cons->annotation;
    term *construct = copy_term(cons);
    while (annot->kind == PI){
        construct = make_app(construct, copy_term(annot->left));
        if(construct->right->name) free(construct->right->name);
        construct->right->name = 0;
        annot = annot->right;
    }
    return construct;
}

term *convert_unnamed(term *t)
{
    if(t->annotation) return t;
    term *type = t;
    term *var = calloc(1, sizeof(term));
    var->kind = VAR;
    var->annotation = type;
    var->name = copy_string("_");
    return var;
}

term *make_eliminator(term *t, term **constructors, int n)
{
    int i;

    term *P = make_variable(append_string(t->name, "_motive"));
    P->annotation = make_pi(make_variable(copy_string("_")), make_type());
    P->annotation->left->annotation = copy_term(t);

    term *fun = calloc(1, sizeof(term));
    term *top = fun;
    fun->name = append_string(t->name, "_elim");
    fun->kind = FUN;
    fun->left = P;

    term *ret = make_variable(append_string(t->name, "_return"));
    ret->kind = ELIM;
    ret->n = t->n;
    ret->cases = calloc(ret->n, sizeof(term*));

    for (i = 0; i < n; ++i) {
        term *cons = constructors[i];
        term *construction = construct_instance(cons);
        free_term(construction);
        //term *annot = copy_term(cons->annotation);

        fun->right = calloc(1, sizeof(term));
        fun = fun->right;
        fun->kind = FUN;
        term *part = make_variable(append_string(cons->name, "_case"));

        if(cons->annotation->kind == IND){
            term *motive = make_variable(0);
            motive->n = i;
            part->annotation = make_app(motive, copy_term(cons));
        } else {
            char buff[256];
            int level = 0;
            term *construction = construct_instance(cons);
            term *annot = copy_term(cons->annotation);
            part->annotation = annot;
            while(annot->kind == PI){
                if(compare_types(annot->left->annotation, t)){
                    term *prev = make_variable(0);
                    term *motive = make_variable(0);
                    motive->n = level+i+1;
                    term *add_type = make_app(motive, prev);
                    term *var = convert_unnamed(add_type);
                    term *swap = annot->right;
                    increment(swap, 1, 0);
                    increment(construction, 1, 0);
                    annot->right = make_pi(var, swap);
                }
                snprintf(buff, 256, "%d", level);
                if(annot->left->name) free(annot->left->name);
                annot->left->name = append_string("_", buff);
                annot = annot->right;
                ++level;
            }

            term *motive = make_variable(0);
            motive->n = level+i;
            term *last = make_app(motive, construction);
            free_term_contents(annot);
            *annot = *last;
            free(last);
        }
        ret->cases[i] = make_variable(0);
        ret->cases[i]->n = n-i;
        fun->left = part;
    }
    fun->right = calloc(1, sizeof(term));
    fun = fun->right;
    fun->kind = FUN;

    term *arg = make_variable(copy_string("arg"));
    arg->annotation = copy_term(t);

    term *motive = make_variable(0);
    motive->n = n+1;
    ret->annotation = make_app(motive, make_variable(0));
    ret->left  = copy_term(motive);
    ret->right = copy_term(arg);

    fun->left = arg;
    fun->right = ret;

    return top;
}

void evaluate_term(term *t, environment *env)
{
    if (!t) return;
    if (t->annotation) evaluate_term(t->annotation, env);
    if (t->kind == DEF){
        evaluate_term(t->right, env);
        free_term(t->right->annotation);
        t->right->annotation = copy_term(t->left->annotation);
    }
    if (t->kind == ELIM) {
        term *arg = t->right;  
        evaluate_term(arg, env);
        if (arg->kind == CONS) {
            term *c = t->cases[arg->n];
            replace_term(t, c);
            evaluate_term(t, env);
        } else if (arg->kind == APP){
            term *base_type = type_infer(arg, env, 0);
            // (branch t t) -> ((branch t) t)
            // (S O)
            // ((ps k) elim)
            // ((ps t1) t2)
            // (((ps t1) t2) elim_t2)
            // ((((ps t1) elim_t1) t2) elimt2)
            term *replace = copy_term(arg);
            term *top = replace;
            while (replace->kind == APP){
                term *next = replace->right;
                if(type_check(next, env, 0, base_type)){
                    term *elim = copy_term(t);
                    free_term(elim->right);
                    elim->right = copy_term(next);
                    term *rec = make_app(replace, elim);
                    replace_term(replace, rec);
                    free_term(elim);
                    free(rec);
                    replace = replace->left;
                }
                if(replace->left->kind != APP){
                    int n = replace->left->n;
                    free_term(replace->left);
                    replace->left = copy_term(t->cases[n]);
                }
                replace = replace->left;
            }

            replace_term(t, top);
            free_term(top);
            free_term(base_type);
            evaluate_term(t, env);
        }
        return;
    }
    if (t->kind == IND){
        // Um, do nothing
    }
    if (t->kind == CONS){
    }
    if (t->kind == VAR){
        term *lookup = 0;
        if (t->name) lookup = get_environment(env, t->name);
        if (lookup){
            if(lookup->kind == DEF){
                lookup = lookup->right;
            }
            replace_term(t, lookup);
            evaluate_term(t, env);
        }
    }
    if (t->kind == FUN){
        evaluate_term(t->left, env);
        evaluate_term(t->right, env);
    }
    if (t->kind == PI){
        evaluate_term(t->left, env);
        evaluate_term(t->right, env);
    }
    if (t->kind == APP){
        evaluate_term(t->left, env);
        evaluate_term(t->right, env);
        if (t->left->kind == FUN){

            term *lam = t->left;
            term *arg = copy_term(t->right);

            replace_term(t, lam->right);

            substitute(t, arg, 0);

            free_term(arg);
            evaluate_term(t, env);
        }
    }
}

term *type_infer(term *t, environment *env, term_list *context)
{
    if(t->kind == ELIM){
        return copy_term(t->annotation);
    }
    if(t->kind == IND){
        return copy_term(t->annotation);
    }
    if(t->kind == CONS){
        return copy_term(t->annotation);
    }
    if(t->kind == VAR){
        if (t->annotation) return copy_term(t->annotation);
        term *l = resolve(t, env, context);
        term *m = copy_term(l);
        if(DEBUG) printf("Before increment:");
        if(DEBUG) print_term(m);
        if(DEBUG) printf("\n");
        increment(m, t->n+1, 0);
        if(DEBUG) printf("increment by %d\n", t->n+1);

        if(DEBUG) printf("Inferred ");
        if(DEBUG) print_term(t);
        if(DEBUG) printf(" has type ");
        if(DEBUG) print_term(m);
        if(DEBUG) printf("\n");

        return m;
    }
    if (t->kind == FUN) {
        term *arg = t->left;
        term *body = t->right;
        evaluate_term(arg, env);
        context = push_term_list(context, arg);
        term* infer = type_infer(body, env, context);
        evaluate_term(infer, env);
        context = pop_term_list(context);
        if(!infer){
            if(DEBUG) printf("Couldn't infer Body type ");
            if(DEBUG) print_term(body);
            if(DEBUG) printf("\n");
            return 0;
        }
        term *pi = make_pi(copy_term(arg), infer);

        if(DEBUG) printf("Inferred ");
        if(DEBUG) print_term(t);
        if(DEBUG) printf(" has type ");
        if(DEBUG) print_term(pi);
        if(DEBUG) printf("\n");

        return pi;
    }
    if(t->kind == APP){
        term *f = t->left;
        term *x = t->right;
        term *pi = type_infer(f, env, context);
        evaluate_term(pi, env);
        if(!pi) return 0;
        if (pi->kind != PI){
            if(DEBUG) print_term(f);
            if(DEBUG) printf(" doesn't have PI type\n");
            free_term(pi);
            return 0;
        }
        term *T = pi->right;
        term *S = pi->left->annotation;
        if (!S){
            if(DEBUG) printf("PI variable doesn't have annotation\n");
            return 0;
        }
        int check = type_check(x, env, context, S);
        if (!check) return 0;
        term *sub = copy_term(T);
        if(DEBUG) printf("Subbing ");
        if(DEBUG) print_term(x);
        if(DEBUG) printf(" in ");
        if(DEBUG) print_term(sub);
        if(DEBUG) printf("\n");
        substitute(sub, x, 0);

        free_term(pi);

        if(DEBUG) printf("Inferred ");
        if(DEBUG) print_term(t);
        if(DEBUG) printf(" has type ");
        if(DEBUG) print_term(sub);
        if(DEBUG) printf("\n");
        return sub;
    }
    if(t->kind == PI){
        term *base = calloc(1, sizeof(term));
        base->kind = TYPE;

        if(DEBUG) printf("Inferred ");
        if(DEBUG) print_term(t);
        if(DEBUG) printf(" has type ");
        if(DEBUG) print_term(base);
        if(DEBUG) printf("\n");

        return base;
    }
    if(t->kind == TYPE){
        term *base = calloc(1, sizeof(term));
        base->kind = TYPE;
        base->n = t->n+1;

        if(DEBUG) printf("Inferred ");
        if(DEBUG) print_term(t);
        if(DEBUG) printf(" has type ");
        if(DEBUG) print_term(base);
        if(DEBUG) printf("\n");

        return base;
    }
    if(t->kind == DEF){
        if(t->left->annotation){

            if(DEBUG)printf("Before: ");
            if(DEBUG)print_term(t->left->annotation);
            if(DEBUG)printf("\nAfter: ");

            evaluate_term(t->left->annotation, env);

            if(DEBUG)print_term(t->left->annotation);
            if(DEBUG)printf("\n");
            if(DEBUG)print_term(t->left->annotation);

            int check = type_check(t->right, env, context, t->left->annotation);
            if(!check){
                term *lt = type_infer(t->left, env, context);
                term *rt = type_infer(t->right, env, context);
                printf("Left: ");
                real_print_term(lt);
                printf("\n");
                printf("Right: ");
                real_print_term(rt);
                printf("\n");
                free_term(lt);
                free_term(rt);
            }
            if(check) return copy_term(t->left->annotation);
        }else{
            term *infer = type_infer(t->right, env, context);    
            t->left->annotation = copy_term(infer);
            return infer;
        }
        return 0;
    }
    return 0;
}

int type_check(term *t, environment *env, term_list *context, term *type)
{
    evaluate_term(type, env);
    if(DEBUG) printf("check ");
    if(DEBUG) print_term(t);
    if(DEBUG) printf(" is type ");
    if(DEBUG) print_term(type);
    if(DEBUG) printf("\n");
    if(DEBUG) printf("Context: ");
    if(DEBUG) print_term_list(context);
    if(t->kind == IND){
        printf("IND always checks");
        return 1;
    }
    if(t->kind == HOLE){
        printf("Hole should have type: ");
        real_print_term_r(type, context);
        printf("\n");
        return 1;
    }
    //VAR, APP, FUN, IND, PI, TYPE, CONS, DEF
    if(type->kind == PI){
        if(t->kind == FUN){
            term *arg_type = type->left->annotation;
            term *arg = t->left;
            evaluate_term(arg, env);
            if(DEBUG) print_term(arg);
            if(DEBUG) printf("\n");

            if(DEBUG) print_term(arg_type);
            if(DEBUG) printf("\n");
            if(arg->annotation){
                if(!compare_types(arg->annotation, arg_type)){
                    if(DEBUG) printf("Arguement doesn't match annotation\n");
                    return 0;
                }
            }else{
                arg->annotation = copy_term(arg_type);
            }
            term *body = t->right;
            term *body_type = type->right;

            context = push_term_list(context, arg);
            int check = type_check(body, env, context, body_type);
            context = pop_term_list(context);
            return check;
        }
        if(t->kind == PI || t->kind == TYPE || t->kind == IND) return 0;
    }else if(type->kind == TYPE){
        if(t->kind == PI || t->kind == TYPE || t->kind == IND) return 1;
        if(t->kind == FUN) return 0;
    } else if (type->kind == VAR){

    } else if (type->kind == APP){
        term *infer = type_infer(t, env, context);
        evaluate_term(infer, env);
        int compare = compare_types(infer, type);
        if(!compare){
            printf("APPs don't match!\n");
            printf("APP Left: ");
            real_print_term(infer);
            printf("\nAPP Right: ");
            real_print_term(type);
            printf("\n");
        }
        free_term(infer);
        return compare;
    } else if (type->kind == IND){
        term *infer = type_infer(t, env, context);
        evaluate_term(infer, env);
        int compare = compare_types(infer, type);
        free_term(infer);
        return compare;
    } else {
        if(DEBUG) printf("Bad: ");
        if(DEBUG) print_term(type);
        if(DEBUG) printf("\n");
        assert(0);
        error("Fuck");
    }

    if (t->kind == DEF) return 0;

    if (t->kind == VAR){
        term *l = resolve(t, env, context);
        term *m = copy_term(l);
        if(!m){
            if(DEBUG) print_term(l);
            if(DEBUG) printf(" No annotation for VAR\n");
        }
        increment(m, t->n+1, 0);
        int compare = compare_types(m, type);
        free_term(m);
        if (compare) return 1;
        if(DEBUG) printf("VAR doesn't match type\n");
        return 0;
    } else if (t->kind == APP){
        term *infer = type_infer(t, env, context);
        evaluate_term(infer, env);
        int compare = compare_types(infer, type);
        free_term(infer);
        if (compare) return 1;
        if(DEBUG) printf("APP doesn't match type\n");
        return 0;
    }
    else{
        error("Shit");
    }
    return 0;
}

typedef struct index_list{
    char *name;
    struct index_list *next;
} index_list;

void debruijn_r(term *t, index_list *list)
{
    if(DEBUG) printf("debruge\n");
    if(DEBUG) print_term(t);
    if(DEBUG) printf("\n");
    if (!t) return;
    if (t->annotation) debruijn_r(t->annotation, list);
    if (t->kind == VAR){
        int count = 0;
        while(list){
            if(DEBUG) printf("...%s...", list->name);
            if(strcmp(list->name, t->name) == 0){
                t->n = count;
                free(t->name);
                t->name = 0;
                break;
            }
            ++count;
            list = list->next;
        }
        if(DEBUG) printf("\n");
        t->n = count;
    }else if (t->kind == DEF){
        debruijn_r(t->left->annotation, list);
        debruijn_r(t->right, list);
    }else if (t->kind == APP){
        debruijn_r(t->left, list);
        debruijn_r(t->right, list);
    }else if (t->kind == FUN || t->kind == PI){
        debruijn_r(t->left->annotation, list);
        index_list *top = calloc(1, sizeof(index_list));
        top->name = t->left->name;
        if(DEBUG) printf("Adding %s to list\n", top->name);

        top->next = list;
        debruijn_r(t->right, top);
        if(DEBUG) printf("Popping %s from list\n", top->name);
        free(top);
    }else if (t->kind == IND){
    }else if (t->kind == ELIM){
        debruijn_r(t->left, list);
        int i;
        for(i = 0; i < t->n; ++i){
            debruijn_r(t->cases[i], list);
        }
    }else if (t->kind == TYPE){
    }
}

void debruijn(term *t)
{
    debruijn_r(t, 0);
}

void (*real_print_term)(term*);
void (*real_print_term_r)(term*, term_list*);

void print_term_fun(term *t, term_list *context)
{
    printf("(fun ");
    term_list *front = context;
    while(t && t->kind == FUN){
        //print_term_r(t->left, context);
        printf("%s", t->left->name);
        context = push_term_list(context, t->left);
        t = t->right;
        printf(" ");
    }
    /*
       if(t->annotation){
       printf(" : ");
       print_term_r(t->annotation, list);
       }
     */
    printf(" => ");

    print_term_r(t, context);
    while(context != front){
        context = pop_term_list(context);
    }

    printf(")");
}

void print_ind(term *t, term_list *context)
{
    printf("%s", t->name);
}

void print_term_r(term *t, term_list *context)
{
    if(!t){
        printf("Null Term");
        return;
    }
    term_list *front = context;
    if (t->kind == VAR){
        /*
           if(t->name && strcmp(t->name, "_")==0){
           print_term_r(t->annotation, front);
           return;
           }
         */
        if(t->name){
            printf("%s", t->name);
        }else{
            term *lookup = get_term_list(context, t->n);
            if(!lookup){
                printf("%d", t->n);
                printf("\n");
                print_term_list(context);
                printf("\n");
            }
            else printf("%s", lookup->name);
        }
        //printf("#%d", t->n);
        if(t->annotation){
            printf(":");
            print_term_r(t->annotation, front);
        }
    }else if (t->kind == ELIM){
        printf("(%s ", t->name);
        int i;
        print_term_r(t->left, context);
        printf(" ");
        for(i = 0; i < t->n; ++i){
            print_term_r(t->cases[i], context);
            printf(" ");
        }
        print_term_r(t->right, context);
        printf(")");
    }else if (t->kind == DEF){
        printf("def ");
        print_term_r(t->left, context);
        printf(" = ");
        print_term_r(t->right, context);
    }else if (t->kind == IND){
        printf("%s", t->name);
    }else if (t->kind == APP){
        printf("(");
        if(t->left->kind == CONS){
            printf("%s", t->left->name);
        }else{
            print_term_r(t->left, context);
        }
        printf(" ");
        if(t->right->kind == CONS){
            printf("%s", t->right->name);
        }else{
            print_term_r(t->right, context);
        }
        printf(")");
    }else if (t->kind == FUN){
        print_term_fun(t, context);
    }else if (t->kind == CONS){
        printf("%s", t->name);
        if(t->annotation){
            printf(":");
            print_term_r(t->annotation, context);
        }
    }else if (t->kind == PI){
        printf("(");
        print_term_r(t->left, context);
        /*
           if(t->left->annotation){
           printf(":");
           print_term_r(t->left->annotation, context);
           }
         */
        printf(" -> ");
        term *next = t;
        term_list *top = context;
        while (next->right->kind == PI){
            context = push_term_list(context, next->left);
            print_term_r(next->right->left, context);
            printf(" -> ");
            next = next->right;
        }
        context = push_term_list(context, next->left);
        print_term_r(next->right, context);
        while(top != context) context = pop_term_list(context);
        printf(")");
    }else if (t->kind == TYPE){
        printf("Type");
    }else if (t->kind == HOLE){
        printf(". ");
    }
}

void print_term(term *t)
{
    print_term_r(t, 0);
}

void print_term_lisp(term *t)
{
		int i;
		if(!t){
				printf("nil");
				return;
		}

    if (t->kind == VAR){
				printf("(VAR ");
    }else if (t->kind == APP){
				printf("(APP ");
    }else if (t->kind == FUN){
				printf("(FUN ");
    }else if (t->kind == IND){
				printf("(IND ");
    }else if (t->kind == PI){
				printf("(PI ");
    }else if (t->kind == TYPE){
				printf("(TYPE ");
    }else if (t->kind == CONS){
				printf("(CONS ");
    }else if (t->kind == DEF){
				printf("(DEF ");
    }else if (t->kind == HOLE){
				printf("(HOLE ");
    }else if (t->kind == ELIM){
				printf("(ELIM ");
    }else{
				printf("(%d ", t->kind);
		}

		if(t->name){
				printf("%s ", t->name);
		}else{
				printf("nil ");
		}

		printf("%d ", t->n);

		print_term_lisp(t->left);
		printf(" ");
		print_term_lisp(t->right);
		printf(" ");
		print_term_lisp(t->annotation);
		if(t->cases){
				for(i = 0; i < t->n; i++){
						printf(" ");
						print_term_lisp(t->cases[i]);
				}
		}
		printf(")");
}

void print_term_lisp_helper(term_list *ctx, int *n)
{
		if(!ctx) return;
		print_term_lisp_helper(ctx->next, n);
		++*n;
		printf("(CTX ");
		print_term_lisp(ctx->value);
		printf(" ");
}

void print_term_lisp_r(term *t, term_list *ctx)
{
		int n;
		print_term_lisp_helper(ctx, &n);
		print_term_lisp(t);
		for(; n > 0; n--) printf(")");
}
