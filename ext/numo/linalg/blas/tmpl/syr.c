#define args_t <%=name%>_args_t

typedef struct {
    enum CBLAS_ORDER order;
    enum CBLAS_UPLO uplo;
    rtype alpha;
} args_t;

#define func_p <%=func_name%>_p

static <%=func_name%>_t func_p = 0;

static void
<%=c_iter%>(na_loop_t *const lp)
{
    dtype *a;
    char *p1;
    ssize_t s1;
    args_t *g;
    blasint n, lda;

    INIT_PTR(lp,0,p1,s1);
    a = (dtype*)NDL_PTR(lp,1);
    g = (args_t*)(lp->opt_ptr);

    n = NDL_SHAPE(lp,0)[0];
    lda = NDL_STEP(lp,1) / sizeof(dtype);

    (*func_p)(g->order, g->uplo, n, g->alpha,
              (dtype*)p1, s1/sizeof(dtype), a, lda);
}

/*<%
 params = [
   vec("x"),
   mat("a",:inplace),
   opt("alpha"),
   opt("uplo"),
   opt("order")
 ].select{|x| x}.join("\n  ")
%>
  @overload <%=name%>( x, [a, alpha:1, uplo:'U', order:'R'] )
  <%=params%>
  @return [<%=class_name%>] return a
<%=description%>

 */
static VALUE
<%=c_func(-1)%>(int argc, VALUE const argv[], VALUE UNUSED(mod))
{
    VALUE     ans;
    VALUE     x, a, alpha;
    narray_t *na1, *na3;
    blasint   nx, na;
    size_t    shape[2];
    ndfunc_arg_in_t ain[3] = {{cT,1},{OVERWRITE,2},{sym_init,0}};
    ndfunc_arg_out_t aout[1] = {{cT,2,shape}};
    ndfunc_t ndf = {<%=c_iter%>, NO_LOOP, 2, 0, ain, aout};

    args_t g;
    VALUE kw_hash = Qnil;
    ID kw_table[3] = {id_alpha,id_order,id_uplo};
    VALUE opts[3] = {Qundef,Qundef,Qundef};

    CHECK_FUNC(func_p,"<%=func_name%>");

    rb_scan_args(argc, argv, "11:", &x, &a, &kw_hash);
    rb_get_kwargs(kw_hash, kw_table, 0, 3, opts);
    alpha   = option_value(opts[0],Qnil);
    g.alpha = RTEST(alpha) ? NUM2DBL(alpha) : 1;
    g.order = option_order(opts[1]);
    g.uplo  = option_uplo(opts[2]);

    GetNArray(x,na1);
    CHECK_DIM_GE(na1,1);
    nx = COL_SIZE(na1); // n

    if (a == Qnil) { // c is not given.
        ndf.nout = 1;
        ain[1] = ain[2];
        a = INT2FIX(0);
        shape[0] = shape[1] = nx;
    } else {
        COPY_OR_CAST_TO(a,cT);
        GetNArray(a,na3);
        CHECK_DIM_GE(na3,2);
        CHECK_SQUARE("a",na3);
        na = COL_SIZE(na3); // n (lda)
        CHECK_SIZE_EQ(na,nx);
    }

    ans = na_ndloop3(&ndf, &g, 2, x, a);

    if (ndf.nout == 1) { // a is not given.
        return ans;
    } else {
        return a;
    }
}

#undef func_p
#undef args_t
