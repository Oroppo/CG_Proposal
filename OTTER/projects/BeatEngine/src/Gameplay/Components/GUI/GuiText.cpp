#include "Gameplay/Components/GUI/GuiText.h"
#include <locale>
#include <codecvt>
#include "Graphics/GuiBatcher.h"
#include "Utils/ImGuiHelper.h"
#include "Utils/JsonGlmHelpers.h"
#include "Gameplay/GameObject.h"
#include "Application/application.h"

std::wstring_convert<std::codecvt_utf8_utf16<wchar_t>> StringConvert;

GuiText::GuiText() :
	IComponent(),
	_text(LR"()"), // The LR and parenthesis tell us it's a unicode string (wide string)
	_color(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)),
	_font(nullptr),
	_textSize(glm::vec2(0.0f)),
	_textScale(1.0f)
{ }

GuiText::GuiText(float perX, float perY, float scale) :
	IComponent(),
	_text(LR"()"), // The LR and parenthesis tell us it's a unicode string (wide string)
	_color(glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)),
	_font(nullptr),
	_textSize(glm::vec2(0.0f)),
	_textScale(1.0f),

	_percentOfScreenX(perX),
	_percentOfScreenY(perY),
	_scale(scale)
{ }

GuiText::~GuiText() = default;

void GuiText::SetColor(const glm::vec4& color) {
	_color = color;
}

const glm::vec4& GuiText::GetColor() const {
	return _color;
}

std::string GuiText::GetText() const {
	return StringConvert.to_bytes(_text);
}

void GuiText::SetText(const std::string& value) {
	SetTextUnicode(StringConvert.from_bytes(value));
}

const std::wstring& GuiText::GetTextUnicode() const {
	return _text;
}

void GuiText::SetTextUnicode(const std::wstring& value) {
	_text = value;
	
	if (_font != nullptr) {
		_textSize = _font->MeausureString(_text, _textScale);
	}
}

const float GuiText::GetTextScale() const {
	return _textScale;
}

void GuiText::SetTextScale(float value) {
	_textScale = value;
}

const Font::Sptr& GuiText::GetFont() const {
	return _font;
}

void GuiText::SetFont(const Font::Sptr& font) {
	_font = font;
	if (_font != nullptr) {
		_textSize = _font->MeausureString(_text, _textScale);
	}
}

void GuiText::Awake() {
	_transform = GetComponent<RectTransform>();
	if (_transform == nullptr) {
		IsEnabled = false;
		LOG_WARN("Failed to find a rect transform for a GUI panel, disabling");
	}
}

void GuiText::RenderGUI()
{
	if (_font != nullptr && !_text.empty()) {
		glm::vec2 position = _transform->GetSize() / 2.0f;
		position -= _textSize / 2.0f;
		GuiBatcher::RenderText(_text, _font, position, _color, _textScale);
	}
}

void GuiText::RenderImGui()
{
	static char buffer[4096];
	std::string ascii = StringConvert.to_bytes(_text);
	memcpy(buffer, ascii.data(), ascii.size());

	if (LABEL_LEFT(ImGui::InputTextMultiline, "Text", buffer, 4096)) {
		_text = StringConvert.from_bytes(buffer);
		if (_font != nullptr) {
			_textSize = _font->MeausureString(_text, _textScale);
		}
	}
	LABEL_LEFT(ImGui::ColorEdit4, "Color", &_color.x);
	if (LABEL_LEFT(ImGui::DragFloat, "Scale", &_textScale, 0.01f)) {
		if (_font != nullptr) {
			_textSize = _font->MeausureString(_text, _textScale);
		}
	}
}

nlohmann::json GuiText::ToJson() const {
	return {
		{ "color", _color },
		{ "text",  _text },
		{ "scale", _textScale },
		{ "font",  _font  ? _font->GetGUID().str() : "null" }
	};
}

void GuiText::Update(float deltaTime) {

	//Get Window Size
	Application& app = Application::Get();
	glm::vec2 windowSize = app.GetWindowSize();

	//Update the GUI to dynamically move to the correct percent position of the screen. For example, an element dead center (50%, 50%) on a 1080p screen will stay centered when rescaled to 4k 
	GetGameObject()->Get<RectTransform>()->SetPosition({ windowSize.x * _percentOfScreenX, windowSize.y * _percentOfScreenY });
	GetGameObject()->Get<GuiText>()->SetTextScale(_scale * (windowSize.y / 1080));

}

GuiText::Sptr GuiText::FromJson(const nlohmann::json& blob) {
	GuiText::Sptr result = std::make_shared<GuiText>();
	result->_color     = JsonGet(blob, "color", result->_color);
	result->_textScale = JsonGet(blob, "scale", 1.0f);
	result->_text      = JsonGet<std::wstring>(blob, "text", LR"()");
	result->_font      = ResourceManager::Get<Font>(Guid(JsonGet<std::string>(blob, "font", "null")));
	result->SetFont(result->_font);
	return result;
}
