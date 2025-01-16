#include "../core/UpdateList.h"
#include "../tiling/TileMap.hpp"
#include "../tiling/RandomNoise.hpp"

#include "../include/imgui/imgui.h"//

bool layersWindow = false;
bool layersShown[MAXLAYER];
int nodesPerLayer[MAXLAYER] = {0};

bool noiseGenWindow = false;

bool colorPickerWindow = false;

bool imguiInitialized = false;

std::vector<bool> nodeWindows;
std::vector<Node *> nodes;

sint debugTextureStart = 0;
Layer debugLayer = 0;

void Checkbox(std::string name, std::function<bool(int)> getter, std::function<void(int, bool)> setter, int input) {
	bool result = getter(input);
	bool changed = result;
	ImGui::Checkbox(name.c_str(), &result);
	if(result != changed)
		setter(input, result);
}

void Text(std::string name, sf::Vector2f value) {
	name += " = (%.3f,%.3f)";
	ImGui::Text(name.c_str(), value.x, value.y);
}

void Text(std::string name, sf::Vector2i value) {
	name += " = (%d,%d)";
	ImGui::Text(name.c_str(), value.x, value.y);
}

void setupNodes() {
	for(Layer layer = 0; layer < layerNames().size(); layer++) {
		int count = 0;
		Node *source = UpdateList::getNode(layer);
		while(source != NULL) {
			sint id = source->getId();
			while(id >= nodeWindows.size()) {
				nodeWindows.push_back(false);
				nodes.push_back(NULL);
			}
			nodes[id] = source;
			count++;
			source = source->getNext();
		}
		nodesPerLayer[layer] = count;
	}
}

void imguiLayersWindow() {
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Layers", &layersWindow);

    for(Layer layer = 0; layer < layerNames().size(); layer++) {
    	ImGui::PushID(layer);

    	if(ImGui::CollapsingHeader(layerNames()[layer].c_str())) {
    		Checkbox("Hidden", UpdateList::isLayerHidden, UpdateList::hideLayer, layer);
    		Checkbox("Paused", UpdateList::isLayerPaused, UpdateList::pauseLayer, layer);
    		Checkbox("Screenspace", UpdateList::isLayerScreenSpace, UpdateList::screenSpaceLayer, layer);
    		Checkbox("Static", UpdateList::isLayerStatic, UpdateList::staticLayer, layer);

    		ImGui::Text("%d Nodes", nodesPerLayer[layer]);

    		if(ImGui::BeginChild("##", ImVec2(400.0f, std::min(200.0f, nodesPerLayer[layer]*20.f+10)), ImGuiChildFlags_Borders, 0)) {
	    		int count = 0;
	    		Node *source = UpdateList::getNode(layer);
	    		while(source != NULL) {
	    			sint id = source->getId();
	    			while(id >= nodeWindows.size()) {
	    				nodeWindows.push_back(false);
	    				nodes.push_back(NULL);
	    			}
	    			nodes[id] = source;
	    			count++;

	    			std::string nodeName = std::to_string(id);
	    			bool window = nodeWindows[id];
	    			if(ImGui::Selectable(nodeName.c_str(), &window))
	                    nodeWindows[id] = window;
					source = source->getNext();
				}
				nodesPerLayer[layer] = count;
			}
			ImGui::EndChild();
    	}
    	ImGui::PopID();
    }

    ImGui::End();
}

void imguiNodeWindow(Node *source) {
	sint id = source->getId();
	std::string nodeName = "Node " + std::to_string(id) + " : " + layerNames()[source->getLayer()];
	bool window = nodeWindows[id];

	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
	ImGui::SetNextWindowPos(ImVec2(ImGui::GetCursorScreenPos().x+510, ImGui::GetCursorScreenPos().y+id*50), ImGuiCond_FirstUseEver);
    ImGui::Begin(nodeName.c_str(), &window);
    nodeWindows[id] = window;

    if(source->getParent() != NULL) {
		sint pid = source->getParent()->getId();
    	std::string parentName = "Parent = " + std::to_string(pid);
    	bool parentWindow = nodeWindows[pid];
    	if(ImGui::Selectable(parentName.c_str(), &parentWindow))
            nodeWindows[pid] = parentWindow;
    } else
		ImGui::Text("Parent = NULL");

	Text("Position", source->getPosition());
	Text("Origin", source->getOrigin());
	Text("Size", source->getSize());
	Text("Scale", source->getScale());

	ImGui::Text("BlendMode = %d", source->getBlendMode());

	int texture = source->getTexture();
	if(texture < textureFiles().size())
		ImGui::Text("Texture = %d (%s)", texture, textureFiles()[texture].c_str());
	else
		ImGui::Text("Texture = %d", texture);

	bool nodeHidden = source->isHidden();
	ImGui::Text("Hidden = ");
	ImGui::SameLine();
	ImGui::Checkbox("##", &nodeHidden);
	source->setHidden(nodeHidden);

	if(source->getTextureRects()->size() > 0) {
		ImGui::Text("Texture Rects = %lu", source->getTextureRects()->size());

        if(ImGui::BeginChild("##", ImVec2(400.0f, 200.0f), ImGuiChildFlags_Borders, 0)) {
            for(TextureRect rect : *source->getTextureRects()) {
                ImGui::Text("(%.3f,%.3f) = (%d,%d)->(%d,%d) / %.3f",
                	rect.px, rect.py, rect.tx, rect.ty, rect.width, rect.height, rect.rotation);
            }
        }
        ImGui::EndChild();
	} else
		ImGui::Text("No Texture Rects");

    ImGui::End();
}

int testSize = 100;
int testDivisions = 4;
bool perlin = true;
noise::module::Perlin testNoise;
ConstIndexer *zeroIndexer;
ConstIndexer *limitIndexer;
NoiseIndexer *noiseIndexer;
RandomIndexer *randomIndexer;
TileMap *debugNoise;

void setupNoise() {
	zeroIndexer = new ConstIndexer(0, testSize, testSize);
	limitIndexer = new ConstIndexer(testDivisions, testSize, testSize);

	noiseIndexer = new NoiseIndexer(zeroIndexer, limitIndexer, &testNoise, 100 / testDivisions);
	randomIndexer = new RandomIndexer(zeroIndexer, limitIndexer, 100 / testDivisions);

	debugTextureStart = std::distance(textureFiles().begin(),
		std::find(textureFiles().begin(), textureFiles().end(), "#DEBUG"));
	debugLayer = std::distance(layerNames().begin(),
		std::find(layerNames().begin(), layerNames().end(), "DEBUG"));

	debugNoise = new TileMap(debugTextureStart+1, 1, 1, noiseIndexer, debugLayer);
	debugNoise->setScale(3,3);
	UpdateList::addNode(debugNoise);
}

void imguiNoiseGenWindow() {
	ImGui::SetNextWindowSize(ImVec2(500, 400), ImGuiCond_FirstUseEver);
    ImGui::Begin("Noise Map Generator", &noiseGenWindow);

	float scale = debugNoise->getScale().x;
	ImGui::SliderFloat("Scale", &scale, 0.0f, 10.0f, "%.3f");
	debugNoise->setScale(scale, scale);

	ImGui::SeparatorText("Random Input");

	bool redraw = false;

	bool _perlin = perlin;
	ImGui::Checkbox("Perlin Noise", &perlin);
	if(_perlin != perlin)
		debugNoise->setIndexer(perlin ? (Indexer*)noiseIndexer : (Indexer*)randomIndexer);

	ImGui::SliderInt("Color Divisions", &testDivisions, 2, 100);
	if(testDivisions != limitIndexer->fallback) {
		testDivisions = 100 / round(100.0 / testDivisions);
		limitIndexer->fallback = testDivisions;
		noiseIndexer->multiplier = 100 / testDivisions;
		randomIndexer->multiplier = 100 / testDivisions;
		redraw = true;
	}

	int seed = testNoise.GetSeed();
	ImGui::SliderInt("Seed", &seed, 0, 100);
	if((int)testNoise.GetSeed()!=seed) {
		testNoise.SetSeed(seed);
		randomIndexer->seed = seed;
		redraw = true;
	}

	if(!perlin)
		ImGui::BeginDisabled();

	ImGui::SeparatorText("Perlin Noise");

	float frequency = testNoise.GetFrequency();
	float persistence = testNoise.GetPersistence();
	float lacunarity = testNoise.GetLacunarity();
	int octaveCount = testNoise.GetOctaveCount();

	ImGui::SliderFloat("Frequency", &frequency, 1, 50);
	ImGui::SliderFloat("Persistence", &persistence, 0, 1);
	ImGui::SliderFloat("Lacunarity", &lacunarity, 0, 10);
	ImGui::SliderInt("OctaveCount", &octaveCount, 1, 30);

	if((float)testNoise.GetFrequency()!=frequency  || (float)testNoise.GetPersistence()!=persistence ||
		(float)testNoise.GetLacunarity()!=lacunarity || testNoise.GetOctaveCount()!=octaveCount) {

		//libnoise test map
		testNoise.SetFrequency(frequency);
		testNoise.SetPersistence(persistence);
		testNoise.SetLacunarity(lacunarity);
		testNoise.SetOctaveCount(octaveCount);
		redraw = true;
	}

	if(!perlin)
		ImGui::EndDisabled();

	//noise::module::Checkerboard checkerNoise;
	//noise::module::Add addNoise;
	//addNoise.SetSourceModule(0, testNoise);
	//addNoise.SetSourceModule(1, checkerNoise);

	debugNoise->setPosition(ImGui::GetWindowPos().x+ImGui::GetWindowSize().x, ImGui::GetWindowPos().y);
	if(redraw) {
		debugNoise->reload();
		redraw = false;
	}

	//
	//

	/*int counters[testDivisions] = {0};
	randomIndexer->mapGrid([&counters, testDivisions](int c, sf::Vector2f pos) {
		counters[c/ (100/testDivisions)]++;
	});
	int sum = 0;
	for(int c : counters) {
		std::cout << c << "\n";
		sum += c;
	}
	std::cout << sum << "\n";*/

    ImGui::End();
}

float col1[4] = { 1.0f, 0.0f, 0.2f, 1.0f };
sf::Vector2i pickPosition;
int pickIndex = 10;
int pickTexture = -1;
float pickTextureScale = 6;

void imguiColorPickerWindow() {
	ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_FirstUseEver);
    ImGui::Begin("Color Picker", &colorPickerWindow);

    ImGui::ColorEdit4("Color", col1);

    if(pickTexture == -1)
		pickTexture = debugTextureStart+3;

	ImGui::SeparatorText("Image Selector");

    int texture = pickTexture;
    ImGui::SliderInt("Texture", &pickTexture, 0, textureFiles().size()-1);
	ImGui::SliderFloat("Scale", &pickTextureScale, 0.0f, 10.0f, "%.3f");
    if(textureFiles()[pickTexture].length()==0 || textureFiles()[pickTexture][0] == '#') {
    	ImGui::End();
    	return;
    }

    ImGui::SeparatorText("Image Picker");

    if(texture != pickTexture)
    	pickPosition = sf::Vector2i(0,0);

    sf::Vector2i size = UpdateList::getTextureSize(pickTexture);
    ImGui::SliderInt("x", &pickPosition.x, 0, size.x-1);
    ImGui::SliderInt("y", &pickPosition.y, 0, size.y-1);

    int index = pickPosition.x + pickPosition.y*size.x;
    ImGui::SliderInt("Index", &index, 0, size.x*size.y-1);

    if(pickIndex != index || texture != pickTexture) {
    	pickPosition.x = index % size.x;
    	pickPosition.y = index / size.x;

    	Color pickColor = UpdateList::pickColor(pickTexture, pickPosition);
	    col1[0] = pickColor.red;
	    col1[1] = pickColor.green;
	    col1[2] = pickColor.blue;
	    col1[3] = pickColor.alpha;
	    pickIndex = index;
	}

	ImGui::Image(UpdateList::getImguiTexture(pickTexture), ImVec2(size.x*pickTextureScale, size.y*pickTextureScale));

    ImGui::End();
}

void imguiShowNode(sint id) {
	nodeWindows[id] = true;
}

void skyrmionImguiMenu() {
	if(ImGui::BeginMenu("Skyrmion")) {
		ImGui::MenuItem("Layers", 0, &layersWindow);
		ImGui::MenuItem("Noise Map Generator", 0, &noiseGenWindow);
		ImGui::MenuItem("Color Picker", 0, &colorPickerWindow);
		ImGui::EndMenu();
	}
}

void skyrmionImgui() {
	if(!imguiInitialized) {
		setupNodes();
		setupNoise();
		imguiInitialized = true;
	}

	if(layersWindow)
		imguiLayersWindow();

	if(noiseGenWindow)
		imguiNoiseGenWindow();
	if(debugNoise != NULL)
		debugNoise->setHidden(!noiseGenWindow);

	if(colorPickerWindow)
		imguiColorPickerWindow();

	for(sint i = 0; i < nodeWindows.size(); i++)
		if(nodeWindows[i] && nodes[i] != NULL)
			imguiNodeWindow(nodes[i]);
}