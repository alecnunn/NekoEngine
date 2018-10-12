#include "PanelHierarchy.h"
#include "Application.h"
#include "GameObject.h"
#include "ModuleScene.h"

#include "ImGui/imgui.h"

PanelHierarchy::PanelHierarchy(char* name) : Panel(name) {}

PanelHierarchy::~PanelHierarchy()
{
}

bool PanelHierarchy::Draw()
{
	ImGui::SetNextWindowSize({ 150,200 }, ImGuiCond_FirstUseEver);
	ImGuiWindowFlags hierarchyFlags = 0;
	hierarchyFlags |= ImGuiWindowFlags_NoFocusOnAppearing;
	hierarchyFlags |= ImGuiWindowFlags_NoSavedSettings;

	if (ImGui::Begin(name, &enabled, hierarchyFlags))
	{
		GameObject* root = App->scene->root;
		if (ImGui::TreeNodeEx(root->GetName()))
		{
			IterateAllChildren(root);
			ImGui::TreePop();
		}
	}
	ImGui::End();

	return true;
}

void PanelHierarchy::IterateAllChildren(GameObject* root)
{
	ImGuiTreeNodeFlags treeNodeFlags = 0;
	if (root->HasChildren())
	{
		for (int i = 0; i < root->GetChildrenLength(); ++i)
		{
			GameObject* child = root->GetChild(i);
			if (child->HasChildren())
			{
				ImGui::Unindent();
				if (ImGui::TreeNodeEx(child->GetName(), treeNodeFlags))
				{
					ImGui::Indent();
					IterateAllChildren(child);
					ImGui::TreePop();
				}
			}
			else
			{
				ImGui::Text(child->GetName());
			}
		}
	}
}
