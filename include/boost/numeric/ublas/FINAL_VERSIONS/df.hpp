//          Copyright(C) Rishabh Arora 2017.
// Distributed under the Boost Software License, Version 1.0.
//    (See accompanying file LICENSE_1_0.txt or copy at
//          http://www.boost.org/LICENSE_1_0.txt)

/** \mainpage BOOST uBLAS Dataframe
 * 
 *
 * \section main_classes Main classes
 * 
 * - \link #boost::numeric::ublas::data_frame                    data_frame \endlink
 * - \link #boost::numeric::ublas::df_column            df_column \endlink
 * - \link #boost::numeric::ublas::data_frame_range                data_frame_range \endlink
 * - \link #boost::numeric::ublas::data_frame_slice               data_frame_slice \endlink
 * - \link #boost::numeric::ublas::data_frame_indirect            data_frame_indirect \endlink
 */

#ifndef _BOOST_UBLAS_DATA_FRAME_
#define _BOOST_UBLAS_DATA_FRAME_	

#include <string>
#include <map>
#include <boost/variant.hpp>
#include <boost/preprocessor/seq/elem.hpp>
#include <boost/preprocessor/seq/for_each_product.hpp>
#include <boost/preprocessor/seq/enum.hpp>
#include <boost/numeric/ublas/vector.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/numeric/ublas/vector_proxy.hpp>
#include <algorithm>
#include "./data_frame_exceptions.hpp"
#include "./vector_proxy.cpp"

// Basic allowed types for vectors
#define INNER_TYPE \
(bool) \
(char) (unsigned char) \
(short) (unsigned short) \
(int) (unsigned int) \
(long) (unsigned long) \
(long long) (unsigned long long) \
(float) (double) (long double) \
(std::string) (std::string*) \

// Vector allowed types for columns
/* WARNING: we can't use all the vector types for the time being because boost::variant has a limit
 * of 20 different types because of another limitation in boost MPL
#define VECTOR_TYPE (scalar_vector)(unit_vector)(vector)(zero_vector)
*/

#define VECTOR_TYPE (vector)

// The following types will be integrated later
/*bounded_vector
compressed_vector
coordinate_vector
c_vector
mapped_vector
mapped_vector_of_mapped_vector
matrix_column_vector
matrix_row_vector
matrix_vector_indirect
matrix_vector_range
matrix_vector_slice
vector_indirect
vector_of_vector
vector_range
vector_slice
*/
/// \PreProcessing for the inner and outer types
// Expand macro
#define VMACRO(r, product) (ublas::BOOST_PP_SEQ_ELEM(0, product)<BOOST_PP_SEQ_ELEM(1, product)>)


// Make a sequence of all types
#define ALL_VARIANT_TYPES BOOST_PP_SEQ_FOR_EACH_PRODUCT(VMACRO, (VECTOR_TYPE) (INNER_TYPE))
// Variant of column types
#define COLUMN_TYPES BOOST_PP_SEQ_ENUM(ALL_VARIANT_TYPES)
// Variant of the inner types
#define COLUMN_DATA_TYPES BOOST_PP_SEQ_ENUM(INNER_TYPE)


namespace boost { namespace numeric { namespace ublas {
	
	// temporary vectors to be used by binary operators.
	vector < bool > A0; vector < char > A1; vector < unsigned char > A2;
	vector < short > A3; vector < unsigned short > A4; vector < int > A5; vector < unsigned int > A6;
	vector < long > A7; vector < unsigned long > A8; vector < long long > A9; vector < unsigned long long > A10;
	vector < float > A11; vector < double > A12; vector < long double > A13;
	vector < std::string > A14; vector < std::string* > A15;
	
	class print_data_frame_column: public boost::static_visitor<void> {
	public:
		template < class T >
		void operator () (const T& x_) const {
			for(size_t i = 0; i < x_.size(); ++i) {
				std::cout << x_ (i) << ' ';
			}
			std::cout << std::endl;
			return;
		}
	};

	//! Represents the column of a dataframe.
	/*! Internally represented by boost::variant < COLUMN_TYPES > .
	 *	Allowed types are: int, double, char, std::string...... (specified in INNER_TYPE).
	 */
	class df_column {
	public:
		//! \brief add 2 df_columns of same type.
		friend df_column operator + (df_column& a, df_column& b);

		//! \brief subtract 2 df_columns of same type.
		friend df_column operator - (df_column& a, df_column& b);
		
		//! \brief add a constant value val to a column of same type.
		template < class T > friend df_column operator + (df_column& a, const T& val);
		
		//! \brief subtract a constant value val from a column of same type.
		template < class T > friend df_column operator - (df_column& a, const T& val);
		
		//! \brief multiply a constant value val to a column of same type.
		template < class T > friend df_column operator * (df_column& a, const T& val);
			
		// ----------------------------
		// Construction and Destruction
		// ----------------------------

		/*! \brief Constructor of df_column.
		 *  By default \c size() == 0.
		 *  type of the column is unset.
		 */ 
		BOOST_UBLAS_INLINE
		df_column (): 
			size_ (0) {}

		/*! \brief Copy Constructor of df_column.
		 *	Copies the vector into data_, T becomes the type of the column.
		 *  \param const lvalue reference to a vector of some type T.
		 */ 
		BOOST_UBLAS_INLINE
		template < class T > 
		df_column (const vector<T>& data) :
			data_ (data), 
			size_ (data.size()) {}

		/*! \brief Copy Constructor of df_column. 
		 *	Copies the col into self.
		 *  \param const lvalue reference to a df_column.
		 */
		BOOST_UBLAS_INLINE 
		df_column (const df_column& col) :
			data_ (col.data()), 
			size_ (col.size()) {}

		/*! \brief Move Constructor of a df_column.
		 *	Moves the vector into data_, T becomes the type of the column.
		 *  \param rvalue reference to a vector <T>, T: type of vector.
		 */
		BOOST_UBLAS_INLINE 
		template < class T > 
		df_column (const vector <T>&& data) {
			data_ = std::move(data);
			size_ = std::move(size_);
		} 

		/*! \brief Move Constructor of df_column. 
		 *	Moves the col into self.
		 *  \param rvalue reference to a df_column.
		 */
		BOOST_UBLAS_INLINE 
		df_column (const df_column&& col) {
			data_ = std::move(col.data());
			size_ = std::move(col.size());
		}

		//! \brief Destructor of df_column. 
		BOOST_UBLAS_INLINE
		~df_column() {}
		
		// --------------------
		// Assignment Operators
		// --------------------

		/*! \brief Copy Assignment Operator of a df_column. 
		 *	Copies the vector into data_, T becomes the type of the column.
		 *  \param const lvalue reference to a vector <T>, T: type of vector.
		 */  
		template < class T >
		BOOST_UBLAS_INLINE 
		df_column& operator = (const vector<T>& data) {
			data_ = data;
			size_ = data.size(); 
			return *this;
		}

		/*! \brief Copy Assignment operator of df_column. 
		 *	Copies the col into self.
		 *  \param const lvalue reference to a df_column.
		 */ 
		BOOST_UBLAS_INLINE
		df_column& operator = (const df_column& col) {
			data_  = col.data();
			size_  = col.size();
			return *this;
		}

		/*! \brief Move Assignemnt Operator of a df_column. 
		 *	Moves the vector into data_, T becomes the type of the column.
		 *  \param rvalue reference to a vector <T>, T: type of vector.
		 */ 
		template < class T >
		BOOST_UBLAS_INLINE 
		df_column& operator = (const vector <T>&& data) {
			data_ = std::move(data);
			size_ = std::move(size_);
			return *this;
		}

		/*! \brief Move Assignment operator of df_column. 
		 *	Copies the col into self.
		 *  \param rvalue reference to a df_column.
		 */
		BOOST_UBLAS_INLINE 
		df_column& operator = (const df_column&& col) {
			data_ = std::move(col.data());
			size_ = std::move(col.size());
			return *this;
		}	

		// ----------------------
		// Manipulation Operators
		// ----------------------

		/*! \brief Addition to df_column.
		 *	Adds x to self if the \c x.type() == \c self.type()  and \c x.size() == \c self.size().
		 *  \param lvalue reference to a df_column.
		 */
		BOOST_UBLAS_INLINE
		df_column operator += (df_column& x) {
			(*this) = (*this) + x;
			return *this;
		}

		/*! \brief Subtraction from df_column.
		 *	Subtracts x from self if the \c x.type() == \c self.type()  and \c x.size() == \c self.size().
		 *  \param lvalue reference to a df_column.
		 */
		BOOST_UBLAS_INLINE
		df_column operator -= (df_column& x) {
			return (*this) = (*this) - x;
		}

		/*! \brief Addition to df_column.
		 *	Adds val to self if the \c x.type() == T.
		 *  \param const lvalue reference to a value.
		 */
		BOOST_UBLAS_INLINE
		template < class T > 
		df_column operator += (const T& val) {
			return (*this) = (*this) + val;
		}

		/*! \brief Subtraction from df_column.
		 *	Subtracts val from self if the \c x.type() == T.
		 *  \param const lvalue reference to a value.
		 */
		BOOST_UBLAS_INLINE
		template < class T > 
		df_column operator -= (const T& val) {
			return (*this) = (*this) - val;
		}

		/*! \brief Multiplication to df_column.
		 *	Multiplies val to self if the \c x.type() == T.
		 *  \param const lvalue reference to a value.
		 */
		BOOST_UBLAS_INLINE
		template < class T > 
		df_column operator *= (const T& val) {
			return (*this) = (*this) * val;
		}

		// ---------
		// Accessors
		// ---------
		
		//! \brief Returns the size of the column vector.
		BOOST_UBLAS_INLINE
		const size_t size() const {
			return size_;
		}	

		//! \brief Returns the 0 - based index of the type of column vector in boost::variant < COLUMN_TYPES >. 
		BOOST_UBLAS_INLINE
		const short type() const {
			return data_.which();
		}
		
		// -----------------
		// Storage Accessors
		// -----------------

		//! \brief Returns the data_ variable.
		BOOST_UBLAS_INLINE
		const boost::variant < COLUMN_TYPES > data() const {
			return data_;
		} 

		/*! \brief Returns the df_column as vector<T>.
 		 *  T should be same as the type of the df_column.
 		 */
		template < class T >
		BOOST_UBLAS_INLINE 
		vector<T>& get() {
			return boost::get<vector<T>>(data_); 
		}

		/*! \brief Returns the i-th element of df_column.
 		 *  T should be same as the type of the df_column.
 		 */
		template < class T > 
		BOOST_UBLAS_INLINE
		T& eval(const size_t& i) {
			return boost::get<vector<T>>(data_)(i);
		}

		//! \brief Print the contents of df_column in a single line.
		BOOST_UBLAS_INLINE
		void print() {
			boost::apply_visitor (print_data_frame_column{}, data_);
		}
		
		// ---------------------
		// Statistical Summaries
		// ---------------------

		//! \brief Returns the minimum element of the column vector. 
		template < class T1, class T2 > 
		BOOST_UBLAS_INLINE 
		T2 Min () {
			T2 Minimum = get<T1> ()(0);
			for(size_t i = 1; i < size_; ++i) {
				if (get <T1> ()(i) < Minimum) {
					Minimum = (T2)get<T1>()(i);
				}
			}	
			return Minimum;
		}

		//! \brief Returns the maximum element of the column vector.
		template < class T1, class T2> 
		BOOST_UBLAS_INLINE 
		T2 Max () {
			T2 Maximum = get<T1>()(0);
			for(size_t i = 1; i < size_; ++i) {
				if (get<T1>()(i) > Maximum) {
					Maximum = (T2) get<T1>()(i);
				}
			}	
			return Maximum;
		}

		//! \brief Returns the mean of the column vector.
		template < class T1, class T2 >
		BOOST_UBLAS_INLINE
		T2 Mean() {
			T2 sum = 0;
			for(size_t i = 0; i < size_; ++i) {
				sum += (T2) get<T1>()(i);
			}
			return (T2) (sum / size_);
		}

		//! \brief Returns the median element of the column vector.
		template < class T1, class T2 >
		BOOST_UBLAS_INLINE
		T2 Median() {
			vector<T1> v = get<T1>();
			std::sort(v.begin(), v.end());
			if (v.size() & 1) {
				return (T2)v[v.size()/2];
			}
			else {
				return ((T2)(v[v.size()/2-1] + v[v.size()/2]) / 2);
			}
		}

		//! \brief Prints Minimum, Maximum, Mean, Median of a column vector.
		BOOST_UBLAS_INLINE
		template <class T1, class T2>
		void summary() {
			std::cout << "Min. : " << Min <T1, T2>()  << ", ";
			std::cout << "Max. : " << Max <T1, T2>()  << ", ";
			std::cout << "Mean : " << Mean <T1, T2>() << ", ";
			std::cout << "Median : " << Median <T1, T2>() << "  ";
			std::cout << std::endl;	 
		}

	private:
		//! \brief Stores the data in df_column. 
		boost::variant < COLUMN_TYPES > data_;
		//! \brief Stores the size of the column vector.
		size_t size_;
	};			
						
	//! \brief Returns the negation of the column if exists. 
	BOOST_UBLAS_INLINE
	df_column operator - (df_column& a) {	
		df_column X;
		switch (a.type()) {
			case 0: 
				A0 = -a.get<bool>();
				return X = A0;
			case 3: 
				A3 = -a.get<short>();
				X = A3;
				return X;
			case 4: 
				A4 = -a.get<unsigned short>();
				X = A4;
				return X;
			case 5: 
				A5 = -a.get<int>();
				X = A5;
				return X;
			case 6: 
				A6 = -a.get<unsigned int>();
				X = A6;
				return X;
			case 7: 
				A7 = -a.get<long>();
				X = A7;
				return X;
			case 8: 
				A8 = -a.get<unsigned long>();
				X = A8;
				return X;
			case 9: 
				A9 = -a.get<long long>();
				X = A9;
				return X;
			case 10: 
				A10 = -a.get<unsigned long long>();
				X = A10;
				return X;
			case 11: 
				A11 = -a.get<float>();
				X = A11;
				return X;
			case 12: 
				A12 = -a.get<double>();
				X = A12;
				return X;
			case 13: 
				A13 = -a.get<long double>();
				X = A13;
				return X;
		}
	}

	//! \brief Returns a df_column as sum of 2 df_columns.
	BOOST_UBLAS_INLINE
	df_column operator + (df_column& a, df_column& b) {	
		df_column X;
		switch (a.type()) {
			case 0: 
				A0 = a.get<bool>() + b.get<bool>();
				X = A0;
				return X;
			case 3: 
				A3 = a.get<short>() + b.get<short>();
				X = A3;
				return X;
			case 4: 
				A4 = a.get<unsigned short>() + b.get<unsigned short>();
				X = A4;
				return X;
			case 5: 
				A5 = a.get<int>() + b.get<int>();
				X = A5;
				return X;
			case 6: 
				A6 = a.get<unsigned int>() + b.get<unsigned int>();
				X = A6;
				return X;
			case 7: 
				A7 = a.get<long>() + b.get<long>();
				X = A7;
				return X;
			case 8: 
				A8 = a.get<unsigned long>() + b.get<unsigned long>();
				X = A8;
				return X;
			case 9: 
				A9 = a.get<long long>() + b.get<long long>();
				X = A9;
				return X;
			case 10: 
				A10 = a.get<unsigned long long>() + b.get<unsigned long long>();
				X = A10;
				return X;
			case 11: 
				A11 = a.get<float>() + b.get<float>();
				X = A11;
				return X;
			case 12: 
				A12 = a.get<double>() + b.get<double>();
				X = A12;
				return X;
			case 13: 
				A13 = a.get<long double>() + b.get<long double>();
				X = A13;
				return X;
		}
	}

	//! \brief Returns a df_column as difference of 2 df_columns.
	BOOST_UBLAS_INLINE
	df_column operator - (df_column& a, df_column& b) {	
		df_column X;
		switch (a.type()) {
			case 0: 
				A0 = a.get<bool>() - b.get<bool>();
				X = A0;
				return X;
			case 3: 
				A3 = a.get<short>() - b.get<short>();
				X = A3;
				return X;
			case 4: 
				A4 = a.get<unsigned short>() - b.get<unsigned short>();
				X = A4;
				return X;
			case 5: 
				A5 = a.get<int>() - b.get<int>();
				X = A5;
				return X;
			case 6: 
				A6 = a.get<unsigned int>() - b.get<unsigned int>();
				X = A6;
				return X;
			case 7: 
				A7 = a.get<long>() - b.get<long>();
				X = A7;
				return X;
			case 8: 
				A8 = a.get<unsigned long>() - b.get<unsigned long>();
				X = A8;
				return X;
			case 9: 
				A9 = a.get<long long>() - b.get<long long>();
				X = A9;
				return X;
			case 10: 
				A10 = a.get<unsigned long long>() - b.get<unsigned long long>();
				X = A10;
				return X;
			case 11: 
				A11 = a.get<float>() - b.get<float>();
				X = A11;
				return X;
			case 12: 
				A12 = a.get<double>() - b.get<double>();
				X = A12;
				return X;
			case 13: 
				A13 = a.get<long double>() - b.get<long double>();
				X = A13;
				return X;
		}
	}

	//! \brief Returns a df_column as sum of a df_column and a constant value.
	BOOST_UBLAS_INLINE
	template < class T > 
	df_column operator + (df_column& a, const T& val) {
		df_column X;
		for(size_t i = 0; i < a.size(); ++i) {
			switch (a.type()) {
				case 0: 
					if (i == 0) {
						A0.resize(a.size());
					}
					A0(i) = a.get<bool>()(i) + val;
					if (i == a.size()-1) {
						X = A0;
						A0.resize(0);
						return X;
					}
				break;
				case 3:
					if (i == 0) {
						A3.resize(a.size());
					} 
					A3(i) = a.get<short>()(i) + val;
					if (i == a.size()-1) {
						X = A3;
						A3.resize(0);
						return X;
					}
				break;
				case 4: 
					if (i == 0) {
						A4.resize(a.size());
					}
					A4(i) = a.get<unsigned short>()(i) + val;
					if (i == a.size()-1) {
						X = A4;
						A4.resize(0);
						return X;
					}
				break;
				case 5: 
					if (i == 0) {
						A5.resize(a.size());
					}
					A5(i) = a.get<int>()(i) + val;
					if (i == a.size()-1) {
						X = A5;
						A5.resize(0);
						return X;
					}
				break;
				case 6:
					if (i == 0) {
						A6.resize(a.size());
					} 
					A6(i) = a.get<unsigned int>()(i) + val;
					if (i == a.size()-1) {
						X = A6;
						A6.resize(0);
						return X;
					}
				break;
				case 7:
					if (i == 0) {
						A7.resize(a.size());
					} 
					A7(i) = a.get<long>()(i) + val;
					if (i == a.size()-1) {
						X = A7;
						A7.resize(0);
						return X;
					}
				break;
				case 8: 
					if (i == 0) {
						A8.resize(a.size());
					}
					A8(i) = a.get<unsigned long>()(i) + val;
					if (i == a.size()-1) {
						X = A8;
						A8.resize(0);
						return X;
					}
				break;
				case 9:
					if (i == 0) {
						A9.resize(a.size());
					} 
					A9(i) = a.get<long long>()(i) + val;
					if (i == a.size()-1) {
						X = A9;
						A9.resize(0);
						return X;
					}
				break;
				case 10:
					if (i == 0) {
						A10.resize(a.size());
					} 
					A10(i) = a.get<unsigned long long>()(i) + val;
					if (i == a.size()-1) {
						X = A10;
						A10.resize(0);
						return X;
					}
				break;
				case 11:
					if (i == 0) {
						A11.resize(a.size());
					} 
					A11(i) = a.get<float>()(i) + val;
					if (i == a.size()-1) {
						X = A11;
						A11.resize(0);
						return X;
					}
				break;
				case 12:
					if (i == 0) {
						A12.resize(a.size());
					} 
					A12(i) = a.get<double>()(i) + val;
					if (i == a.size()-1) {
						X = A12;
						A12.resize(0);
						return X;
					}
				break;
				case 13:
					if (i == 0) {
						A13.resize(a.size());
					} 
					A13(i) = a.get<long double>()(i) + val;
					if (i == a.size()-1) {
						X = A13;
						A13.resize(0);
						return X;
					}
				break;
			}
		}
	}

	//! \brief Returns a df_column as sum of a df_column and a constant value.
	BOOST_UBLAS_INLINE
	template < class T > 
	df_column operator + (const T& val, df_column& a) {
		return a + val;
	}

	//! \brief Returns a df_column as difference of a df_column and a constant value.
	BOOST_UBLAS_INLINE
	template < class T > 
	df_column operator - (df_column& a, const T& val) {
		return a + (-val);
	}

	//! \brief Returns a df_column as product of a df_column and a constant value.
	BOOST_UBLAS_INLINE	
	template < class T > 
	df_column operator * (df_column& a, const T& val) {
		df_column X;
		switch (a.type()) {
			case 0: 
				A0 = a.get<bool>() * val;
				X = A0;
				return X;
			case 3: 
				A3 = a.get<short>() * val;
				X = A3;
				return X;
			case 4: 
				A4 = a.get<unsigned short>() * val;
				X = A4;
				return X;
			case 5: 
				A5 = a.get<int>() * val;
				X = A5;
				return X;
			case 6: 
				A6 = a.get<unsigned int>() * val;
				X = A6;
				return X;
			case 7: 
				A7 = a.get<long>() * val;
				X = A7;
				return X;
			case 8: 
				A8 = a.get<unsigned long>() * val;
				X = A8;
				return X;
			case 9: 
				A9 = a.get<long long>() * val;
				X = A9;
				return X;
			case 10: 
				A10 = a.get<unsigned long long>() * val;
				X = A10;
				return X;
			case 11: 
				A11 = a.get<float>() * val;
				X = A11;
				return X;
			case 12: 
				A12 = a.get<double>() * val;
				X = A12;
				return X;
			case 13: 
				A13 = a.get<long double>() * val;
				X = A13;
				return X;
		}
	}

	//! \brief Returns a df_column as product of a df_column and a constant value.
	BOOST_UBLAS_INLINE
	template < class T > 
	df_column operator * (const T& val, df_column& a) {
		return a * val;
	}

	//! \brief Returns \c true if \c y.size() == \c x.size() and \c y.get<T> == \c x.get<T>() else \c false
	BOOST_UBLAS_INLINE
	bool operator == (df_column& y, df_column& x) {
		if (y.type() != x.type()) {
			return false;
		}
		else if(y.size() != x.size()) {
			return false;
		}
		for(size_t i = 0; i < x.size(); ++i) {
			switch (y.type()) {
				case 0: if((y.get<bool>()(i) != x.get<bool>()(i)) ) {
					return false;
				}
				break;
				case 1: if((y.get<char>()(i) != x.get<char>()(i)) ) {
					return false;
				}
				break;
				case 2: if((y.get<unsigned char>()(i) != x.get<unsigned char>()(i)) ) {
					return false;
				}
				break;
				case 3: if((y.get<short>()(i) != x.get<short>()(i)) ) {
					return false;
				}
				break;
				case 4: if((y.get<unsigned short>()(i) != x.get<unsigned short>()(i)) ) {
					return false;
				}
				break;
				case 5: if((y.get<int>()(i) != x.get<int>()(i)) ) {
					return false;
				}
				break;
				case 6: if((y.get<unsigned int>()(i) != x.get<unsigned int>()(i)) ) {
					return false;
				}
				break;
				case 7: if((y.get<long>()(i) != x.get<long>()(i)) ) {
					return false;
				}
				break;
				case 8: if((y.get<unsigned long>()(i) != x.get<unsigned long>()(i)) ) {
					return false;
				}
				break;
				case 9: if((y.get<long long>()(i) != x.get<long long>()(i)) ) {
					return false;
				}
				break;
				case 10: if((y.get<unsigned long long>()(i) != x.get<unsigned long long>()(i)) ) {
					return false;
				}
				break;
				case 11: if((y.get<float>()(i) != x.get<float>()(i)) ) {
					return false;
				}
				break;
				case 12: if((y.get<double>()(i) != x.get<double>()(i)) ) {
					return false;
				}
				break;
				case 13: if((y.get<long double>()(i) != x.get<long double>()(i)) ) {
					return false;
				}
				break;
				case 14: if((y.get<std::string>()(i) != x.get<std::string>()(i)) ) {
					return false;
				}
				break;
				case 15: if((y.get<std::string*>()(i) != x.get<std::string*>()(i)) ) {
					return false;
				}
				break;
			}
		}
		return true;
	}

	//! \brief Returns \c false if \c y.size() == \c x.size() and \c y.get<T> == \c x.get<T>() else \c true
	BOOST_UBLAS_INLINE
	bool operator != (df_column& y, df_column& x) {
		return !(x == y);
	}


	BOOST_UBLAS_INLINE
	void remove (vector<std::string>& v, size_t idx) {
	    for (size_t i = idx; i < v.size() - 1; i++) {
	        v(i) = v(i + 1);
	    }
	    v.resize(v.size() - 1);
	    return;
	}

	//! Represents a dataframe.
	/*! Internally represented as map < string, df_column > and a vector < string> to store the headers.
	 *	Allowed types are: int, double, char, std::string...... (specified in INNER_TYPE).
	 */
	class data_frame {
	public:
		
		friend data_frame operator + (data_frame& a, data_frame& b);
		friend data_frame operator - (data_frame& a, data_frame& b);
		template < class T > friend data_frame operator + (data_frame& a, const T& val);
		template < class T > friend data_frame operator - (data_frame& a, const T& val);
		template < class T > friend data_frame operator * (data_frame& a, const T& val);
		 
		// construction and destruction
		
		/*! \brief Constructor of data_frame.
		 *  By default \c nrow() == 0 and ncol() == 0.
		 *  type of the columns is unset.
		 */ 
		BOOST_UBLAS_INLINE 
		data_frame () :
			ncol_ (0),
			nrow_ (0) {}

		/*! \brief Constructor of data_frame.
		 *  \param: const lvalue reference to headers(vector<std::string>).
		 *  \param: const lvalue reference to data(vector<df_column>).
		 */ 
		BOOST_UBLAS_INLINE 
		data_frame (const vector<std::string>& headers, const vector<df_column>& data) {
			try {
				// exception for unequal headers and columns
				if (headers.size() != data.size()) {
					throw inconsistent_arguments();
				}

				ncol_ = headers.size();
				nrow_ = (ncol_ > 0) ? data(0).size() : 0; 

				// exception for unequal no. of rows in data_frame columns
				for(size_t i = 1; i < ncol_; ++i) {
					if (data (i).size() != nrow_) {
						throw differing_rows();
					}
				}	

				// initialise the data_ members
				column_headers_ = headers;	
				
				for(size_t i = 0; i < ncol_; ++i) {
					// if a column_name already exists throw an error
					if ( (i != 0) && (data_.find(headers(i))) != data_.end())  {
						throw same_header();
					}	
					data_.insert({headers(i), data(i)});
				}

			}
			catch (std::exception &e) {
				std::terminate();
			}
		}

		//! Destructor of data_frame.
		BOOST_UBLAS_INLINE
		~data_frame() {}

		// ---------------- 
		// column accessors 
		// ---------------- 

		/*! \brief Access operator of data_frame.
		 *  creates the column (with the given header) if doesn't exist. 
		 *  \param column_header to be accessed.
		 *  \return Requested df_column.
		 */ 
		BOOST_UBLAS_INLINE 
		df_column& operator[](const std::string& header) {
			// given column name already exists
			// return the particular column
			if(data_.find(header) != data_.end()) {
				return data_ [header];
			}
			// create the column (will be furthur used by = operator in df_column)
			column_headers_.resize(ncol_ + 1);
			column_headers_(ncol_) = header;
			++ncol_;
			return data_ [header];
		}
		
		/*! \brief Access operator of data_frame.
		 *  creates the column (if index is valid and non-existent) if doesn't exist. 
		 *  valid-index == \c ncol()
		 *  \param column-index to be accessed.
		 *  \return Requested df_column.
		 */ 
		BOOST_UBLAS_INLINE 
		df_column& operator[] (const size_t& i) {
			try {
				// given column name already exists
				// return the particular column
				if(i < ncol_) {
					return data_ [column_headers_ (i)];
				}
				else if(i > ncol_) {
					throw holes();
				} 
				// custom name if no name is set by default
				column_headers_.resize(ncol_ + 1);
				column_headers_(ncol_) = default_name(ncol_); 
				++ncol_;
				return data_ [column_headers_(i)];
			}
			catch(std::exception &e) {
				std::terminate();
			}
		}
		
		/*! \brief Access operator of data_frame. 
		 *  \param column_header to be accessed.
		 *  \param T(template parameter): type of the column data. 
		 *  \return vector<T> version of the column.	
		 */ 
		template < class T > 
		BOOST_UBLAS_INLINE
		vector<T>& column (const std::string& header) {
			try {
				if (data_.find(header) == data_.end()) {
					throw undefined_column_header();
				}
				return data_[header].get<T>();
			}
			catch(std::exception& e) {
				std::terminate();
			}
		}

		/*! \brief Access operator of data_frame. 
		 *  \param column index to be accessed.
		 *  \param T(template parameter): type of the column data. 
		 *  \return vector<T> version of the column.	
		 */ 
		template < class T > 
		BOOST_UBLAS_INLINE
		vector<T>& column(const size_t& i) {
			return data_[column_headers_ (i)].get<T>();
		}

		// ------------- 
		// row accessors 
		// ------------- 

		/*! brief Row Access operator of data_frame.
		 *  \param row index to be accessed.
		 *  Returns the row as vector < int, double ...... >.
		 */
		vector < boost::variant < COLUMN_DATA_TYPES > > operator () (const size_t row) {
			vector < boost::variant < COLUMN_DATA_TYPES > > ret(ncol_);
			for(size_t i = 0; i < ncol_; ++i) {
				switch((*this)[i].type()) {
					case 0: 	
						ret(i) = (*this)[i].eval<bool>(row);
						break;
					case 1: 
						ret(i) = (*this)[i].eval<char>(row);
						break;
					case 2: 
						ret(i) = (*this)[i].eval<unsigned char>(row);
						break;
					case 3: 
						ret(i) = (*this)[i].eval<short>(row);
						break;
					case 4: 
						ret(i) = (*this)[i].eval<unsigned short>(row);
						break;
					case 5: 
						ret(i) = (*this)[i].eval<int>(row);
						break;
					case 6: 
						ret(i) = (*this)[i].eval<unsigned int>(row);
						break;
					case 7: 
						ret(i) = (*this)[i].eval<long>(row);
						break;
					case 8: 
						ret(i) = (*this)[i].eval<unsigned long>(row);
						break;
					case 9: 
						ret(i) = (*this)[i].eval<long long>(row);
						break;
					case 10: 
						ret(i) = (*this)[i].eval<unsigned long long>(row);
						break;
					case 11: 
						ret(i) = (*this)[i].eval<float>(row);
						break;
					case 12: 
						ret(i) = (*this)[i].eval<double>(row);
						break;
					case 13: 
						ret(i) = (*this)[i].eval<long double>(row);
						break;
					case 14: 
						ret(i) = (*this)[i].eval<std::string>(row);
						break;
					case 15: 
						ret(i) = (*this)[i].eval<std::string*>(row);
						break;
				} 
			}
			return ret;
		} 

		// ------------ 
		// erase column 
		// ------------ 

		/*! \brief erase_column operator of data_frame.
		 *  erase the column(\c i) and restructure the data_frame.  
		 *  \param column index to be erased.
		 */ 
		BOOST_UBLAS_INLINE 
		void erase_column(const size_t i) {
			try {
				if ( (i >= column_headers_.size()) || (i < 0)) {
					throw bad_index();
				}
				/// \i is valid so delete the column[$i]
				data_.erase(column_headers_[i]);
				remove(column_headers_, i);
				--ncol_;
			}
			catch(std::exception &e){
				std::terminate();
			}
		}
		
		/*! \brief erase_column operator of data_frame.
		 *  erase the column(\c header) and restructure the data_frame.  
		 *  \param column header to be accessed.
		 */ 
		BOOST_UBLAS_INLINE 
		void erase_column(const std::string& header) {
			try {
				if (data_.find(header) == data_.end()) {
					throw undefined_column_header();
				}
				// name is valid so delete the column[$name]
				data_.erase(header);
				for(size_t i = 0; i < column_headers_.size(); i++) {
					if (header == column_headers_(i)) {
						remove(column_headers_, i);
						break;
					}
				}
				--ncol_;
			}
			catch(std::exception &e) {
				std::terminate();
			}
		}

		// ---------- 
		// add column 
		// ---------- 

		/*! \brief add_column operator of data_frame.
		 *  adds column at the end of the data_frame columns.  
		 *  \param column header
		 *  \param const lvalue reference to the column data in the form of df_column. 
		 */ 		
		BOOST_UBLAS_INLINE
		data_frame& add_column(const std::string& header, const df_column& col) {
			try {
				for(size_t i = 0; i < ncol_; ++i) {
					if (column_headers_(i) == header) {
						same_header(); 
					}
				}
				column_headers_.resize(ncol_ + 1);
				column_headers_ (ncol_) = header;
				++ncol_;
				data_ [header] = col;
				return *this;
			}
			catch (std::exception& e) {
				std::terminate();
			}
		}

		/*! \brief prints the data in the data_frame.
		 *  Print Format: [column header]: data(0) data(1) .......  
		 */ 		
		BOOST_UBLAS_INLINE 
		void print() {
			for(size_t i = 0; i < ncol_; ++i) {
				std::cout << "[" << column_headers_(i) << "]" << ": ";
				data_ [column_headers_(i)].print();
			}
		}
											
		// -----------------
		// Storage Accessors
		// -----------------

		//! \brief Returns the headers of data_frame as vector<std::string>.
		BOOST_UBLAS_INLINE
		vector<std::string>& headers() {
			return column_headers_;
		}

		// ------------------------
		// Random Access Containers
		// ------------------------

		//! \brief Returns the no. of columns in data_frame.
		BOOST_UBLAS_INLINE
		const size_t ncol() const {
			return ncol_;
		}

		//! \brief Returns the no. of rows in data_frame.
		BOOST_UBLAS_INLINE
		const size_t nrow() const {
			return nrow_;
		}

		//! \brief Returns the column header at i-th index.
		BOOST_UBLAS_INLINE
		const std::string colname(const size_t& i) {
			return column_headers_(i);
		}

		// ---------------------
		// Statistical Summaries
		// ---------------------

		/*! \brief prints the column statistics in the data_frame.
		 *  Print Format: [column header]: data(0) data(1) .......  
		 *  Doesn't calculate statistics of non numerical types.
		 */ 
		BOOST_UBLAS_INLINE
		void summary() {
			for(size_t i = 0; i < ncol_; ++i) {
				std::cout << "[" << column_headers_(i) << "]" << ": ";
				 switch(data_[column_headers_(i)].type()) {
					case 0: 	
						data_[column_headers_(i)].summary <bool, long double >(); break;
					case 3: 
						data_[column_headers_(i)].summary <short, long double >(); break;
					case 4: 
						data_[column_headers_(i)].summary <unsigned short, long double >(); break;
					case 5: 
						data_[column_headers_(i)].summary <int, long double >(); break;
					case 6: 
						data_[column_headers_(i)].summary <unsigned int, long double >(); break;
					case 7: 
						data_[column_headers_(i)].summary <long, long double >(); break;
					case 8: 
						data_[column_headers_(i)].summary <unsigned long, long double >(); break;
					case 9: 
						data_[column_headers_(i)].summary <long long, long double >(); break;
					case 10: 
						data_[column_headers_(i)].summary <unsigned long long, long double >(); break;
					case 11: 
						data_[column_headers_(i)].summary <float, long double >(); break;
					case 12: 
						data_[column_headers_(i)].summary <double, long double >(); break;
					case 13: 
						data_[column_headers_(i)].summary <long double, long double >(); break;
					default: 
						std::cout << "Statistical Summaries not defined" << std::endl;
				}
			}
		}

		//! \brief Adds another data_frame to self.
		BOOST_UBLAS_INLINE
		data_frame operator += (data_frame& df_) {
			return (*this) = (*this) + df_;
		}

		//! \brief Subtracts another data_frame from self.
		BOOST_UBLAS_INLINE
		data_frame operator -= (data_frame& df_) {
			return (*this) = (*this) - df_;
		}

		//! \brief Adds a constant value to self.
		BOOST_UBLAS_INLINE
		template < class T > 
		data_frame operator += (const T& val) {
			return (*this) = (*this) + val;
		}

		//! \brief Subtracts constant value from self.
		BOOST_UBLAS_INLINE
		template < class T > 
		data_frame operator -= (const T& val) {
			return (*this) = (*this) - val;
		}

		//! \brief Multiplies constant value to self.
		BOOST_UBLAS_INLINE
		template < class T > 
		data_frame operator *= (const T& val) {
			return (*this) = (*this) * val;
		}
		
		/*! \brief Update the column header of a column.
		 *	\param std::string to be updated, std::string updated to.
		 */
		BOOST_UBLAS_INLINE 
		void set_col_header(const std::string& a, const std::string& b) {
			try {
				if (data_.find(a) == data_.end()) {
					throw column_header_mismatch();
				}
				for(size_t i = 0; i < column_headers_.size(); ++i) {
					if (column_headers_ (i) == a) {
						column_headers_(i) = b;
						break;
					}
				}
				data_[b] = data_[a];
				data_.erase(a);
			}
			catch (std::exception& e) {
				std::terminate();
			}
		}

		/*! \brief Update the column header of a column.
		 *	\param index of the column to be updated, std::string updated to.
		 */
		BOOST_UBLAS_INLINE 
		void set_col_header(const size_t col, const std::string& b) {
			data_[b] = data_[column_headers_(col)];
			data_.erase(column_headers_(col));
			column_headers_(col) = b;
		}

	private:
		//! \brief Stores data.
		std::map < std::string, df_column > data_;
		//! \brief Stores sizes.
		size_t ncol_, nrow_;
		//! \brief Stores column headers.
		vector < std::string> column_headers_;
		
		//! \brief Returns the default column name.
		//! used when the column name is not set by the user.s
		BOOST_UBLAS_INLINE
		std::string default_name(size_t i) {
			return ("Col-" + boost::lexical_cast<std::string>(i));
		}
	};


	
	//! \brief Returns the negation of the data_frame if exists. 
	data_frame operator - (data_frame& a) {
		vector<std::string> header(a.ncol());
		vector<df_column> col(a.ncol());
		for(size_t i = 0; i < a.ncol(); ++i) {
			header(i) = a.colname(i);
			col(i) = - a[i];
		}
		return data_frame(header, col);
	}
	
	//! \brief Returns a data_frame as sum of 2 data_frames.
	data_frame operator + (data_frame& a, data_frame& b) {
		try {
			if (a.ncol() != b.ncol()) {
				throw unequal_columns();
			}
			else if (a.nrow() != b.nrow()) {
				throw unequal_rows();
			}
			vector<std::string> header(a.ncol());
			vector<df_column> col(a.ncol());
			for(size_t i = 0; i < a.ncol(); ++i) {
				if (a.colname(i) != b.colname(i)) {
					throw column_header_mismatch();
				}
				header(i) = a.colname(i);
				col(i) = a[i] + b[i];
			}
			return data_frame(header, col);
		}
		catch (std::exception& e) {
			std::terminate();
		}
	}

	//! \brief Returns a data_frame as difference of 2 data_frames.
	data_frame operator - (data_frame& a, data_frame& b) {
		try {
			if (a.ncol() != b.ncol()) {
				throw unequal_columns();
			}
			else if (a.nrow() != b.nrow()) {
				throw unequal_rows();
			}
			vector<std::string> header(a.ncol());
			vector<df_column> col(a.ncol());
			for(size_t i = 0; i < a.ncol(); ++i) {
				if (a.colname(i) != b.colname(i)) {
					throw column_header_mismatch();
				}
				header(i) = a.colname(i);
				col(i) = a[i] - b[i];
			}
			return data_frame(header, col);
		}
		catch (std::exception& e) {
			std::terminate();
		}
	}

	//! \brief Returns a data_frame as sum of a data_frame and a constant value.
	template < class T >
	data_frame operator + (data_frame& a, const T& val) {
		vector<std::string> header(a.ncol());
		vector<df_column> col(a.ncol());
		for(size_t i = 0; i < a.ncol(); ++i) {
			header(i) = a.colname(i);
			col(i) = a[i] + val;
		}
		return data_frame(header, col);
	}

	//! \brief Returns a data_frame as sum of a data_frame and a constant value.
	template < class T > 
	data_frame operator + (const T& val, data_frame& a) {
		return a + val;
	}

	//! \brief Returns a data_frame as difference of a data_frame and a constant value.
	template < class T > 
	data_frame operator - (data_frame& a, const T& val) {
		return a + (-val);
	}

	//! \brief Returns a data_frame as product of a data_frame and a constant value.
	template < class T >
	data_frame operator * (data_frame& a, const T& val) {
		vector<std::string> header(a.ncol());
		vector<df_column> col(a.ncol());
		for(size_t i = 0; i < a.ncol(); ++i) {
			header(i) = a.colname(i);
			col(i) = a[i] * val;
		}
		return data_frame(header, col);
	}

	//! \brief Returns a data_frame as product of a data_frame and a constant value.
	template < class T > 
	data_frame operator * (const T& val, data_frame& a) {
		return a * val;
	}

	//! \brief Returns \c true if (for all columns of a and b) \c y.size() == \c x.size() and \c y.get<T> == \c x.get<T>() else \c false
	bool operator == (data_frame& a, data_frame& b) {
		if (a.ncol() != b.ncol()) {
			return false;
		}
		else if(a.nrow() != b.nrow()) {
			return false;
		}
		for(size_t i = 0; i < a.ncol(); ++i) {
			if (a.headers()(i) != b.headers()(i)) {
				return false;
			}
			else if(a[i] != b[i]) {
				return false;
			}
		}
		return true;
	}

	//! \brief Returns \c false if (for all columns of a and b) \c y.size() == \c x.size() and \c y.get<T> == \c x.get<T>() else \c true
	bool operator != (data_frame& a, data_frame& b) {
		return !(a == b);
	}

	// -----------------
	// dataframe proxies
	// -----------------

	/*! \brief Represents data_frame column range class
	 *	Builds a range on columns of another data_frame.
	 *	allows access and modifications operations
	 */
	class data_frame_range {
	public:
		typedef vector<std::string>::size_type size_type;
		typedef vector<std::string>::difference_type difference_type;
		typedef basic_range<size_type, difference_type> range_type;

		//! \brief: default constructor of data_frame_range.
		BOOST_UBLAS_INLINE
		data_frame_range();

		/*! \brief constructor of data_frame_range.
		 *	\param pointer to base data_frame (over which the proxy needs to be built).
		 *	\param Boost.range over vector<std::string>(column headers).
		 */
		BOOST_UBLAS_INLINE
		data_frame_range (data_frame* df, const range_type& range): 
			column_headers_(df->headers(), range) {
				df_ = df;
		}

		//! \brief: destructor of data_frame_range.
		~data_frame_range() {}

		//! \brief Returns a new data_frame containing copy of columns referenced by the given vector_range.
		BOOST_UBLAS_INLINE
		data_frame DataFrame () {
			vector<std::string> v1(column_headers_.size());
			vector<ublas::df_column> v2(column_headers_.size());
			for(size_t i = 0; i < column_headers_.size(); ++i) {
				v1(i) = column_headers_(i);
				v2(i) = (*df_)[v1(i)];
			}
			return data_frame(v1, v2);
		} 

		/*! \brief Access operator of data_frame_range.
		 * 	\param column index to be accessed.
		 *  Returns reference to the column in the base data_frame.
		 */
		BOOST_UBLAS_INLINE
		df_column& operator [] (const size_t& i) {
			return (*df_)[column_headers_[i]];
		}
		
		//! \brief Returns the size of the range (number of elements in the range).
		BOOST_UBLAS_INLINE
		const size_t size() const {
			return column_headers_.size();
		}

		//! \brief Prints the range similar to a data_frame.
		BOOST_UBLAS_INLINE
		void print() {
			for(auto header: column_headers_) {
				std::cout << "[" << header << "]: ";
				(*df_)[header].print();
			}
			return;
		}

		//! \brief Adds all the values in range by \c val.
		template < class T >
		BOOST_UBLAS_INLINE
		data_frame_range operator += (const T& val) {
			for(auto header: column_headers_) {
				(*df_)[header] += val;
			}
			return (*this);
		}

		//! \brief Subtracts \c val from all the values in range.
		template < class T >
		BOOST_UBLAS_INLINE
		data_frame_range operator -= (const T& val) {
			for(auto header: column_headers_) {
				(*df_)[header] -= val;
			}
			return (*this);		
		}

		//! \brief Multiplies all the values in range by \c val.
		template < class T >
		BOOST_UBLAS_INLINE
		data_frame_range operator *= (const T& val) {
			for(auto header: column_headers_) {
				(*df_)[header] *= val;
			}
			return (*this);			
		}

	private:
		//! \brief Represents the base data_frame.
		data_frame *df_;
		//! \brief Range over the column of data_frame.
		vector_range < vector <std::string> > column_headers_;
	};

	/*! \brief Represents data_frame column slice class
	 *	Builds a slice on columns of another data_frame.
	 *	Allows access and modifications operations
	 */
	class data_frame_slice {
	public:
		typedef vector<std::string>::size_type size_type;
		typedef vector<std::string>::difference_type difference_type;
		typedef basic_slice<size_type, difference_type> slice_type;

		//! \brief: default constructor of data_frame_slice.
		BOOST_UBLAS_INLINE
		data_frame_slice ();

		/*! \brief constructor of data_frame_slice.
		 *	\param pointer to base data_frame (over which the proxy needs to be built).
		 *	\param Boost.slice over vector<std::string>(column headers).
		 */
		BOOST_UBLAS_INLINE
		data_frame_slice (data_frame* df, const slice_type& slice): 
			column_headers_(df->headers(), slice) {
				df_ = df;
		}

		//! \brief: destructor of data_frame_slice.
		~data_frame_slice() {}

		//! \brief Returns a new data_frame containing copy of columns referenced by the given vector_slcie.
		BOOST_UBLAS_INLINE
		data_frame DataFrame () {
			vector<std::string> v1(column_headers_.size());
			vector<ublas::df_column> v2(column_headers_.size());
			for(size_t i = 0; i < column_headers_.size(); ++i) {
				v1(i) = column_headers_(i);
				v2(i) = (*df_)[v1(i)];
			}
			return data_frame(v1, v2);
		} 

		/*! \brief Access operator of data_frame_slice.
		 * 	\param column index to be accessed.
		 *  Returns reference to the column in the base data_frame.
		 */
		BOOST_UBLAS_INLINE
		df_column& operator [] (const size_t& i) {
			return (*df_)[column_headers_[i]];
		}

	
		//! \brief Returns the size of the slice (number of elements in the slice).
		BOOST_UBLAS_INLINE
		const size_t size() const {
			return column_headers_.size();
		}

		//! \brief Prints the slice similar to a data_frame.
		BOOST_UBLAS_INLINE
		void print() {
			for(auto header: column_headers_) {
				std::cout << "[" << header << "]: ";
				(*df_)[header].print();
			}
			return;
		}

		//! \brief Adds all the values in slice by \c val.
		template < class T >
		BOOST_UBLAS_INLINE
		data_frame_slice operator += (const T& val) {
			for(auto header: column_headers_) {
				(*df_)[header] += val;
			}
			return (*this);
		}

		//! \brief Subtracts \c val from all the values in slice.
		template < class T >
		BOOST_UBLAS_INLINE
		data_frame_slice operator -= (const T& val) {
			for(auto header: column_headers_) {
				(*df_)[header] -= val;
			}
			return (*this);
		}

		//! \brief Multiplies all the values in slice by \c val.
		template < class T >
		BOOST_UBLAS_INLINE
		data_frame_slice operator *= (const T& val) {
			for(auto header: column_headers_) {
				(*df_)[header] *= val;
			}
			return (*this);
		}
	private:
		//! \brief Represents the base data_frame.
		data_frame *df_;
		//! \brief Slice over the column of data_frame.
		vector_slice < vector <std::string> > column_headers_;
	};

	/*! \brief Represents data_frame column indirect class
	 *	Builds an indirect vector on columns of another data_frame.
	 *	allows access and modifications operations
	 */
	class data_frame_indirect {
	public:

		//! \brief: default constructor of data_frame_indirect.
		BOOST_UBLAS_INLINE
		data_frame_indirect();

		/*! \brief constructor of data_frame_indirect.
		 *	\param pointer to base data_frame (over which the proxy needs to be built).
		 *	\param indirect_array over vector<std::string>(column headers).
		 */
		BOOST_UBLAS_INLINE
		data_frame_indirect(data_frame* df, const indirect_array<> ia):
		 	column_headers_(df->headers(), ia){
		 	df_ = df;
		} 

		//! \brief Returns a new data_frame containing copy of columns referenced by the given vector_slcie.
		BOOST_UBLAS_INLINE
		data_frame DataFrame () {
			vector<std::string> v1(column_headers_.size());
			vector<df_column> v2(column_headers_.size());
			for(size_t i = 0; i < column_headers_.size(); ++i) {
				v1(i) = column_headers_(i);
				v2(i) = (*df_)[v1(i)];
			}
			return data_frame(v1, v2);
		} 

		//! \brief: destructor of data_frame_indirect.
		~data_frame_indirect() {}

		/*! \brief Access operator of data_frame_indirect.
		 * 	\param column index to be accessed.
		 *  Returns reference to the column in the base data_frame.
		 */
		BOOST_UBLAS_INLINE
		df_column& operator [] (const size_t& i) {
			return (*df_)[column_headers_[i]];
		}
	
		//! \brief Returns the size of the indirect_data_frame (number of elements in the indirect data_frame).
		BOOST_UBLAS_INLINE
		const size_t size() const {
			return column_headers_.size();
		}

		//! \brief Prints the data_frame_indirect similar to a data_frame.
		BOOST_UBLAS_INLINE
		void print() {
			for(auto header: column_headers_) {
				std::cout << "[" << header << "]: ";
				(*df_)[header].print();
			}
			return;
		}

		//! \brief Adds all the values in data_frame_indirect by \c val.
		template < class T >
		BOOST_UBLAS_INLINE
		data_frame_indirect operator += (const T& val) {
			for(auto header: column_headers_) {
				(*df_)[header] += val;
			}
			return (*this);
		}

		//! \brief Subtracts \c val from all the values in data_frame_indirect.
		template < class T >
		BOOST_UBLAS_INLINE
		data_frame_indirect operator -= (const T& val) {
			for(auto header: column_headers_) {
				(*df_)[header] -= val;
			}
			return (*this);			
		}

		//! \brief Multiplies all the values in data_frame_indirect by \c val.
		template < class T >
		BOOST_UBLAS_INLINE
		data_frame_indirect operator *= (const T& val) {
			for(auto header: column_headers_) {
				(*df_)[header] *= val;
			}
			return (*this);
		}

	private: 
		//! \brief Represents the base data_frame.
		data_frame *df_;
		//! \brief vector_indirect over the column of data_frame.
		vector_indirect < vector < std::string> > column_headers_;
	};
}}}

#endif
