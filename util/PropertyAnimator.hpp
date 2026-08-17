#pragma once

#include <functional>
#include "../core/Node.h"

class PropertyAnimationBase {
public:
	virtual bool next(float delta) { return true; }
};

template <typename T>
class PropertyAnimation : public PropertyAnimationBase {
public:
	float maxTime;
	float time;

	std::function<void(T)> setFunc;
	T start;
	T end;

	PropertyAnimation(T* _value, T _end, float _time) {
		maxTime = _time;
		time = 0;

		start = *_value;
		end = _end;

		setFunc = [_value](T v) {
			*_value = v;
		};
	}

	PropertyAnimation(std::function<void(T)> _setFunc, T _start, T _end, float _time) {
		maxTime = _time;
		time = 0;

		start = _start;
		end = _end;

		setFunc = _setFunc;
	}

	bool next(float delta) override {
		time += delta;
		setFunc(lerp(start, end, time/maxTime));

		return time >= maxTime;
	}
};

class PropertyAnimator : public UNode {
private:
	std::vector<PropertyAnimationBase *> running;

public:
	PropertyAnimator(int layer=0) : UNode(layer) {}

	template <typename T>
	void lerp(T* _value, T _end, float _time) {
		running.push_back(new PropertyAnimation(_value, _end, _time));
	}

	template <typename T>
	void lerp(std::function<void(T)> _setFunc, T _start, T _end, float _time) {
		running.push_back(new PropertyAnimation(_setFunc, _start, _end, _time));
	}

	void update(double delta) {
		for(auto it = running.begin(); it != running.end();) {
			if(it[0]->next(delta))
				it = running.erase(it);
			else
				++it;
		}
	}
};

static PropertyAnimator ANIMATOR;