#ifndef UTORRENT_INVARIANT_CHECK
#define UTORRENT_INVARIANT_CHECK

// the invariant checker limits transfer rates to ~1mB/s (from 6)
#ifndef DISABLE_INVARIANT_CHECK
#define DISABLE_INVARIANT_CHECK
#endif

namespace aux_
{

	struct checker_base
	{
		checker_base(): m_prevent_check(false) {}
		virtual ~checker_base() {}
		void do_nothing() const {}
		void dont_check_on_exit() const { m_prevent_check = true; }
		mutable bool m_prevent_check;		
	};

	template<class T>
	struct invariant_checker : checker_base
	{
		invariant_checker(T& instance)
			: m_instance(instance)
		{
			m_instance.check_invariant();
		}

		virtual ~invariant_checker()
		{
			if (m_prevent_check) return;
			m_instance.check_invariant();
		}

		T& m_instance;
	};

	template<class T>
	invariant_checker<T> make_invariant_checker(T& inst)
	{ return invariant_checker<T>(inst); }

}

#if ((!defined NDEBUG) && (!defined DISABLE_INVARIANT_CHECK))
#define INVARIANT_CHECK aux_::checker_base const& _checker = aux_::make_invariant_checker(*this); _checker.do_nothing();
#else
// this expression translates to nothing, while
// it still requires a ; at the end of it and
// cannot interfere with the code following it
// in any way.
#define INVARIANT_CHECK do {} while(false)
#endif

#endif
