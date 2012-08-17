#pragma once


// Namespace for the project
namespace bats {

/**
 * Classifier for allied players. Groups together common classifying checks into this
 * class, such as checking if the an allied is expanding.
 * @author Matteus Magnusson <matteus.magnusson@gmail.com>
 */
class AlliedClassifier {
public:
	/**
	 * Destructor
	 */
	virtual ~AlliedClassifier();

	/**
	 * Returns the instance of AlliedClassifier.
	 * @return instance of AlliedClassifier.
	 */
	static AlliedClassifier* getInstance();

	/**
	 * Checks if the any allied player currently is expanding
	 * @return true if an allied player is expanding
	 */
	bool isExpanding() const;

private:
	/**
	 * Private constructor to enforce singleton usage.
	 */
	AlliedClassifier();

	/**
	 * Disallow copy constructor to enforce singleton usage. Will
	 * generate a compile or link error when used.
	 * @param nonCopyable non copyable object
	 */
	AlliedClassifier(const AlliedClassifier& nonCopyable);

	/**
	 * Disallow assignment operator to enforce singleton usage. Will
	 * generate a compile or link error when used.
	 * @param nonCopyable non copyable object
	 * @return non Copyable object
	 */
	AlliedClassifier& operator=(const AlliedClassifier& nonCopyable);

	static AlliedClassifier* msInstance;
};
}