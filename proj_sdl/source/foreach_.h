/*
 * Basic foreach macro and variants, copied and based on Qt.
 *
 */

#ifndef foreach_defined
#define foreach_defined

#if defined(__cplusplus)
#ifdef __clang__
    struct QForeachContainerBase {};
    template <typename T> class QForeachContainer : public QForeachContainerBase {
        public:
        inline QForeachContainer(T& t): c(t), brk(0), i(c.begin()), e(c.end()) {}
        T& c;
        mutable int brk;
        mutable typename T::iterator i, e;
        inline bool condition() const { return (!brk++ && i != e); }
    };

    template <typename T> class QConstForeachContainer : public QForeachContainerBase {
        public:
        inline QConstForeachContainer(const T& t): c(t), brk(0), i(c.begin()), e(c.end()) {}
        const T c;
        mutable int brk;
        mutable typename T::const_iterator i, e;
        inline bool condition() const { return (!brk++ && i != e); }
    };

    template <typename T> inline T *qForeachPointer(T &) { return 0; }
    template <typename T> inline QForeachContainer<T> qForeachContainerNew(T& t) { return QForeachContainer<T>(t); }
    template <typename T> inline const QForeachContainer<T> *qForeachContainer(QForeachContainerBase *base, T *)
        { return static_cast<const QForeachContainer<T> *>(base); }

    template <typename T> inline T *qForeachPointer(const T &) { return 0; }
    template <typename T> inline QForeachContainer<T> qForeachContainerNew(const T& t) { return QForeachContainer<T>(t); }
    template <typename T> inline const QForeachContainer<T> *qForeachContainer(const QForeachContainerBase *base, const T *)
        { return static_cast<const QForeachContainer<T> *>(base); }

    #define FRM_FOREACH_CURR(container) qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->i
    #define FRM_FOREACH(variable, container) \
        for (const QForeachContainerBase &_container_ = qForeachContainerNew(container);                    \
            qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->condition();            \
            ++qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->i)                    \
                for (variable = *qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->i; \
                    qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->brk;            \
                    --qForeachContainer(&_container_, true ? 0 : qForeachPointer(container))->brk)
#else
	template <typename T> class FrmForeachContainer {
		public:
			inline FrmForeachContainer(T& t) : c(t), brk(0), i(c.begin()), e(c.end()) { }
			T& c;
			int brk;
			typename T::iterator i, e;
	};

    template <typename T> class FrmForeachContainerFrom {
        public:
            inline FrmForeachContainerFrom(T& t, unsigned int start) : c(t), brk(0), i(c.begin()+start), e(c.end()) { }
            T& c;
            int brk;
            typename T::iterator i, e;
    };

    template <typename T> class FrmRForeachContainer {
	public:
		inline FrmRForeachContainer(T& t, unsigned int start = 0) : c(t), brk(0), i(c.rbegin()+start), e(c.rend()) { }
		T& c;
		int brk;
		typename T::reverse_iterator i, e;
	};

    #define FRM_FOREACH_CURR(container) _iterator_.i
	#define FRM_FOREACH(variable, container)									\
		for (FrmForeachContainer<__typeof__(container)> _iterator_(container);	\
			!_iterator_.brk && _iterator_.i != _iterator_.e;					\
			__extension__  ({ ++_iterator_.brk; ++_iterator_.i; }))			\
			for (variable = *_iterator_.i;; __extension__ ({--_iterator_.brk; break;}))

	#define FRM_RFOREACH(variable, container)									\
		for (FrmForeachContainer<__typeof__(container)> _container_(container);	\
			!_container_.brk && _container_.i != _container_.e;					\
			__extension__  ({ ++_container_.brk; ++_container_.i; }))			\
			for (variable = *_container_.i;; __extension__ ({--_container_.brk; break;}))

	#define foreach_		FRM_FOREACH
	#define rforeach_		FRM_RFOREACH

	#define FRM_FOREACH_FROM(variable, container, start)									\
		for (FrmForeachContainerFrom<__typeof__(container)> _container_(container, start);	\
			!_container_.brk && _container_.i != _container_.e;                             \
			__extension__  ({ ++_container_.brk; ++_container_.i; }))                       \
			for (variable = *_container_.i;; __extension__ ({--_container_.brk; break;}))

	#define foreach_from_   FRM_FOREACH_FROM


	template <typename T> class FrmConstForeachContainer {
		public:
			inline FrmConstForeachContainer(T& t) : c(t), brk(0), i(c.begin()), e(c.end()) { }
			T& c;
			int brk;
			typename T::const_iterator i, e;
	};

	#define FRM_CONST_FOREACH(variable, container)                                      \
		for (FrmConstForeachContainer<__typeof__(container)> _container_(container);	\
			!_container_.brk && _container_.i != _container_.e;                         \
			__extension__  ({ ++_container_.brk; ++_container_.i; }))                   \
			for (variable = *_container_.i;; __extension__ ({--_container_.brk; break;}))
	
	#define cforeach_       FRM_CONST_FOREACH
#endif

    #define foreach_		FRM_FOREACH
    #define foreach_current FRM_FOREACH_CURR

#endif

#endif /* foreach_defined */
