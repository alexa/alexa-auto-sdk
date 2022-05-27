/*
 * Copyright 2022 Amazon.com, Inc. or its affiliates. All Rights Reserved.
 *
 * Licensed under the Apache License, Version 2.0 (the "License").
 * You may not use this file except in compliance with the License.
 * A copy of the License is located at
 *
 *     http://aws.amazon.com/apache2.0/
 *
 * or in the "license" file accompanying this file. This file is distributed
 * on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either
 * express or implied. See the License for the specific language governing
 * permissions and limitations under the License.
 */
package com.amazon.alexa.auto.templateruntime.listtemplate;

import android.view.LayoutInflater;
import android.view.View;
import android.view.ViewGroup;
import android.widget.TextView;

import androidx.annotation.NonNull;
import androidx.recyclerview.widget.RecyclerView;

import com.amazon.alexa.auto.aacs.common.ListTemplate;
import com.amazon.alexa.auto.templateruntime.R;

import java.util.Comparator;

public class ListTemplateAdapter extends RecyclerView.Adapter<ListTemplateAdapter.ViewHolder> {
    private final ListTemplate mListTemplate;
    private static int MAX_ITEM_COUNT = 10;
    private int maxCharsLeftTextField;

    public ListTemplateAdapter(ListTemplate listTemplate) {
        this.mListTemplate = listTemplate;
        this.maxCharsLeftTextField =
                listTemplate.getListItems()
                        .stream()
                        .max(Comparator.comparing(listItem -> listItem.getLeftTextField().length()))
                        .get()
                        .getLeftTextField()
                        .length();
    }

    @NonNull
    @Override
    public ViewHolder onCreateViewHolder(@NonNull ViewGroup parent, int viewType) {
        View view = LayoutInflater.from(parent.getContext()).inflate(R.layout.list_item, parent, false);

        return new ViewHolder(view, this.maxCharsLeftTextField);
    }

    @Override
    public void onBindViewHolder(@NonNull ViewHolder holder, int position) {
        holder.leftTextField.setText(mListTemplate.getListItems().get(position).getLeftTextField());
        holder.rightTextField.setText(mListTemplate.getListItems().get(position).getRightTextField());
    }

    @Override
    public int getItemCount() {
        return ((mListTemplate.getListItems().size() <= MAX_ITEM_COUNT) ? mListTemplate.getListItems().size()
                                                                        : MAX_ITEM_COUNT);
    }

    public static class ViewHolder extends RecyclerView.ViewHolder {
        public TextView leftTextField;
        public TextView rightTextField;

        public ViewHolder(View view, int leftTextFieldMaxChars) {
            super(view);

            leftTextField = (TextView) view.findViewById(R.id.leftTextField);
            leftTextField.setWidth((int) (leftTextFieldMaxChars
                    * (view.getResources().getDimensionPixelSize(R.dimen.list_template_card_list_item_textSize)
                            / view.getResources().getDisplayMetrics().density)));
            rightTextField = (TextView) view.findViewById(R.id.rightTextField);
        }
    }
}
