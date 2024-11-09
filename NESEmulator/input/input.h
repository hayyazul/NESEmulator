// input.h - An easier interface to use to get inputs that abstracts away from SDL. This is meant to be a singleton.

#include <map>
#include <SDL.h>

// The first two key states are "static" states; they can be held in that state.
// The other two last only one frame (NOTE: I might change this behavior)
enum KeyState {
	NEUTRAL,  // Keys which haven't been released last frame and is in a neutral position.
	HELD,  // Keys which haven't been pressed last frame and are being held.
	PRESSED,  // Keys which have been pressed last frame.
	RELEASED,  // Keys which have been unpressed last frame.
	NA  // Not a valid keystate; used as a placeholder or similarly to NULL.
};

class Input {
public:
	Input();
	~Input();

	/* void updateInput
	Gets an SDL event and using it updates its stored input values. Returns whether an input was detected.
	*/
	bool updateInput();

	/* void printKeyStates
	A debugger function; outputs all the key states except for the one
	specified in excludedState.
	*/
	void printKeyStates(KeyState excludedState=NA);

	/* bool getQuit
	Returns whether the user has quit or not.
	*/
	bool getQuit();

private:
	std::map<SDL_Scancode, KeyState> keyStates;
	bool quit;

	/* void staticKeyUpdate
	Updates keys in a pressed or released state to their held or neutral counterparts.
	*/
	void staticKeyUpdate();

	/* void updateKeyStates
	Updates the internal keystates given the scancode and whether it was pressed or released.
	 
	@params
	SDL_Scancode key
	bool pressed - true means the key has been pressed, false means it has been released.
	*/
	void updateKeyState(SDL_Scancode key, bool pressed);
};