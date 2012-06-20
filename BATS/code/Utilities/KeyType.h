#pragma once

namespace utilities
{

template <typename T>
class KeyHandler;

/**
* Key type for a specific type
* @author Matteus Magnusson <matteus.magnusson@gmail.com>
*/
template <typename T>
class KeyType
{
public:
	/**
	* Default Constructor for Key Type with a key
	* @param key the key value, default INVALID_KEY
	*/
	inline explicit KeyType() : mKey(INVALID_KEY) {}

	/**
	* Assignment operator
	* @param key assigns a new key value to the key
	*/
	inline void operator=(const KeyType<T>& key) {mKey = key.mKey;}

	/**
	* Equality operator
	* @param key the right-handed key to compare with
	* @return true if the keys are equal
	*/
	inline bool operator==(const KeyType<T>& key) const {return key.mKey == mKey;}

	/**
	* Differ operator
	* @param key the right-handed key to compare with
	* @return true if the keys differ
	*/
	inline bool operator!=(const KeyType<T>& key) const {return key.mKey != mKey;}

	/**
	* Returns true if the key is less than the right key
	* @param key the right-handed key to compare with
	* @return true if the left key is less than the right key
	*/
	inline bool operator<(const KeyType<T>& key) const {return mKey < key.mKey;}

	/**
	* Returns true if the key is less or equal to the right key
	* @param key the right-handed key to compare with
	* @return true if the left key is less or equal to the right key
	*/
	inline bool operator<=(const KeyType<T>& key) const {return mKey <= key.mKey;}

	/**
	* Returns true if the key have a higher value than the right key
	* @param key the right-handed key to compare with
	* @return true if the left key have a higher value than the right key
	*/
	inline bool operator>(const KeyType<T>& key) const {return mKey > key.mKey;}

	/**
	* Returns true if the key have a higher or equal value than the right key
	* @param key the right-handed key to compare with
	* @return true if the left key have a higher or equal value than the right key
	*/
	inline bool operator>=(const KeyType<T>& key) const {return mKey >= key.mKey;}

	/**
	 * Returns true if the key is invalid, equals to testing key == INVALID_KEY
	 * @return true if the keys is invalid.
	 */
	inline bool isInvalid() const {return (*this) == INVALID_KEY;}

	/**
	 * Convert the key to an int. Although we don't want this feature in general,
	 * it's very useful for indexing in vectors, and thus very fast.
	 * @return the integer value of the key
	 */
	inline operator int() const {return mKey;}

	static const KeyType<T> INVALID_KEY;

private:
	/**
	* Constructor for Key Type with a key
	* @param key the key value, default INVALID_KEY
	*/
	inline explicit KeyType(int key) : mKey(key) {}

	friend class KeyHandler<T>;

	int mKey;
};

template<typename T> const KeyType<T> KeyType<T>::INVALID_KEY(-1);
}