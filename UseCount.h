class UseCount
{
	public:
		UseCount();
		UseCount(const UseCount &);
		~UseCount();
		bool only();
		bool reattach(const UseCount &);
	private:
		int *p;
		UseCount & operator = (const UseCount &);
}
