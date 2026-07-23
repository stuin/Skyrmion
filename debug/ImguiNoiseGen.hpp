#include "../core/UpdateList.h"
#include "../tiling/ColorMap.hpp"
#include "../tiling/NoiseIndexer.hpp"
#include "../tiling/MathIndexers.hpp"

#include "../include/imgui/imgui.h"//

class ImguiNoiseGen : public UNode {
private:
	bool open = false;

	int testSize = 100;
	int testDivisions = 4;
	std::array<int, 100> distributionCounters = {0};

	bool perlin = true;
	ConstIndexer *zeroIndexer;
	ConstIndexer *limitIndexer;
	NoiseIndexer *noiseIndexer;
	RandomIndexer *randomIndexer;
	ColorMap *debugNoise;

public:
	ImguiNoiseGen(int debugLayer) : UNode(debugLayer) {
		zeroIndexer = new ConstIndexer(0, testSize, testSize);
		limitIndexer = new ConstIndexer(100, testSize, testSize);

		noiseIndexer = new NoiseIndexer(zeroIndexer, limitIndexer, 0, NOISEPerlin, 100 / testDivisions);
		randomIndexer = new RandomIndexer(zeroIndexer, limitIndexer, 100 / testDivisions);

		debugNoise = new ColorMap(noiseIndexer, percentColorFunc(100), debugLayer);
		//debugNoise->setScale(0.6,0.6);
		UpdateList::addNode(debugNoise);

		UpdateList::addUNode(this);
		UpdateList::addListener(this, EVENT_IMGUI);
	}

	void showWindow() {
		ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	    ImGui::Begin("Noise Map Generator", &open);

		float scale = debugNoise->getScale().x;
		ImGui::SliderFloat("Scale", &scale, 1.0f, 10.0f, "%.3f");
		debugNoise->setScale(scale, scale);

		ImGui::SeparatorText("Random Input");

		bool redraw = false;

		bool _perlin = perlin;
		ImGui::Checkbox("FastNoiseLite", &perlin);
		if(_perlin != perlin) {
			debugNoise->setIndexer(perlin ? (Indexer*)noiseIndexer : (Indexer*)randomIndexer);
			redraw = true;
		}

		ImGui::SliderInt("Color Divisions", &testDivisions, 2, 100);
		if(testDivisions != limitIndexer->fallback) {
			testDivisions = 100 / round(100.0 / testDivisions);
			limitIndexer->fallback = testDivisions;
			noiseIndexer->multiplier = 100 / testDivisions;
			randomIndexer->multiplier = 100 / testDivisions;
			redraw = true;
		}

		int seed = noiseIndexer->getSeed();
		ImGui::SliderInt("Seed", &seed, 0, 100);
		if((int)noiseIndexer->getSeed()!=seed) {
			noiseIndexer->setSeed(seed);
			randomIndexer->setSeed(seed);
			redraw = true;
		}

		if(!perlin)
			ImGui::BeginDisabled();

		ImGui::SeparatorText("FastNoiseLite");

		const char* types[] = { "OpenSimplex2", "OpenSimplex2S", "Cellular", "Perlin", "ValueCubic", "Value" };
		int noiseType = noiseIndexer->getNoiseType();
		ImGui::Combo("Noise Type", &noiseType, types, NOISEMAX);

		float frequency = noiseIndexer->getFrequency();
		float persistence = noiseIndexer->getPersistence();
		int octaveCount = noiseIndexer->getOctaveCount();
		//float lacunarity = noiseIndexer->GetLacunarity();

		ImGui::SliderFloat("Frequency", &frequency, 0, 10);
		ImGui::SliderFloat("Persistence", &persistence, 0, 1);
		ImGui::SliderInt("OctaveCount", &octaveCount, 1, 30);
		//ImGui::SliderFloat("Lacunarity", &lacunarity, 0, 10);

		if(noiseIndexer->getFrequency()!=frequency  || noiseIndexer->getPersistence()!=persistence
			|| noiseIndexer->getOctaveCount()!=octaveCount || noiseIndexer->getNoiseType()!=noiseType) {

			noiseIndexer->setFrequency(frequency);
			noiseIndexer->setOctaves(octaveCount, persistence);
			noiseIndexer->setNoiseType(noiseType);

			//libnoise test map
			redraw = true;
		}

		if(!perlin)
			ImGui::EndDisabled();

		//noise::module::Checkerboard checkerNoise;
		//noise::module::Add addNoise;
		//addNoise.SetSourceModule(0, testNoise);
		//addNoise.SetSourceModule(1, checkerNoise);

		debugNoise->setSPosition(ImGui::GetWindowPos().x+ImGui::GetWindowSize().x, ImGui::GetWindowPos().y);
		if(redraw) {
			debugNoise->reload();
			redraw = false;

			for(int i = 0; i < testDivisions; i++)
				distributionCounters[i] = 0;

	        int _testDivisions = testDivisions;
	        std::array<int, 100> *_distributionCounters = &distributionCounters;
			(perlin ? (Indexer*)noiseIndexer : (Indexer*)randomIndexer)->
				mapGrid([_distributionCounters, _testDivisions](int c, Vector2f pos) {
					int i = c / (100/_testDivisions);
					if(i >= 0 && i < 100)
						(*_distributionCounters)[i]++;
			});
		}

		ImGui::SeparatorText("Noise Distribution");
		ImGui::Text("Expected Value: %d", testSize*testSize/testDivisions);

		for(int i = 0; i < testDivisions; i++)
			ImGui::Text("%d", distributionCounters[i]);

	    ImGui::End();
	}

	void recieveEvent(Event event) {
		if(event.type == EVENT_IMGUI && event.down) {
			ImGui::MenuItem("Noise Map Generator", 0, &open);
		} else if(event.type == EVENT_IMGUI) {
			if(open)
				showWindow();
			if(debugNoise != NULL)
				debugNoise->setHidden(!open);
		}
	}
};