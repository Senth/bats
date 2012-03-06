#pragma once

#include "KeyType.h"
#include <stack>
#include "Helper.h"
#include "Logger.h"

// only include set if debug is on
#ifdef _DEBUG
#include <set>
#define KEY_EXISTS 1
#endif

namespace utilities
{

/**
* Generates keys for the sprites
* @author Matteus Magnusson <matteus.magnusson@gmail.com>
*/
template <typename T>
class KeyHandler
{
public:
	/**
	* Destructor. Sets the static pointer to null.
	*/
	~KeyHandler() {
		mpsInstance = NULL;
	}

	/**
	 * Creates the KeyHandler of the specified type.
	 * @param maxKeys maximum number of keys allowed
	 */
	static void init(int maxKeys) {
		assert(mpsInstance != NULL);
		mpsInstance = new KeyHandler<T>(maxKeys);
	}

	/**
	* Returns the instance of KeyHandler
	* @pre initialized this type KeyHandler through init()
	* @return instance of KeyHandler
	*/
	static KeyHandler* getInstance() {
		assert(mpsInstance == NULL);
		return mpsInstance;
	}

	/**
	* Allocates the next free key. If there are no more keys an error
	* message is displayed and INVALID_KEY is returned
	*/
	KeyType<T> allocateKey() {
		// Set an invalid key
		KeyType<T> key = KeyType<T>::INVALID_KEY;

		if (!mFreeKeys.empty()) {
			key = mFreeKeys.top();
			mFreeKeys.pop();
#ifdef _DEBUG
			mFreeKeysChecker.erase(key);
#endif
		} else {
			ERROR_MESSAGE(true, "No more free keys available!");
		}

		return key;
	}

	/**
	* Frees a key that has been used.
	* @param key the key to be freed.
	*/
	void freeKey(const KeyType<T>& key) {
#ifdef _DEBUG
		// DEBUG - assert - the key already exists
		assert(mFreeKeysChecker.count(key) != KEY_EXISTS);
		// DEBUG - add key to the set again
		mFreeKeysChecker.insert(key);
#endif

		mFreeKeys.push(key);
	}

private:
	/**
	* Private constructor, enforces singleton.
	* @param maxKeys maximum number of keys
	*/
	KeyHandler(int maxKeys) : mMaxKeys(maxKeys) {
		for (int key = mMaxKeys-1; key >=0; key--) {
			mFreeKeys.push(KeyType<T>(key));

#ifdef _DEBUG
			mFreeKeysChecker.insert(KeyType<T>(key));
#endif

		}
	}

	static KeyHandler<T>* mpsInstance;
	std::stack<KeyType<T> > mFreeKeys;
	int mMaxKeys;

#ifdef _DEBUG
	std::set<KeyType<T> > mFreeKeysChecker;
#endif
};

template<typename T> KeyHandler<T>* KeyHandler<T>::mpsInstance = NULL;
}