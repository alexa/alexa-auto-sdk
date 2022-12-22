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

package com.amazon.aacstelephony;

import org.json.JSONArray;
import org.json.JSONException;
import org.json.JSONObject;

import java.util.ArrayList;
import java.util.List;
import java.util.UUID;

/**
 * Models a conversation and returns JSON representation.
 */
public class Conversation {
    private String mId;
    private List<String> mParticipants;
    private ArrayList<Message> mMessages;

    public Conversation() {
        mId = UUID.randomUUID().toString();
        mParticipants = new ArrayList<>();
        mMessages = new ArrayList<>();
    }

    public synchronized void addMessage(String id, String text, String time, String phone, List<String> participants) {
        // Save participants
        for (String participant : participants) {
            if (!mParticipants.contains(participant)) {
                mParticipants.add(participant);
            }
        }

        Message newMessage = new Message(id, text, time, phone);
        // Avoid adding messages with same id
        removeMessage(id);
        mMessages.add(newMessage);
    }

    public synchronized void removeMessage(String id) {
        mMessages.removeIf(message -> message.getId().equals(id));
    }

    public boolean containsParticipants(List<String> participants) {
        return mParticipants.size() == participants.size() && mParticipants.containsAll(participants);
    }

    public String getId() {
        return mId;
    }

    public synchronized int size() {
        return mMessages.size();
    }

    public synchronized JSONObject toJson() throws JSONException {
        JSONObject conversation = new JSONObject();
        conversation.put("id", mId);
        JSONArray otherParticipants = new JSONArray();
        for (String phoneNumber : mParticipants) {
            JSONObject entry = new JSONObject();
            entry.put("address", phoneNumber);
            entry.put("addressType", "PhoneNumberAddress");
            otherParticipants.put(entry);
        }
        conversation.put("otherParticipants", otherParticipants);

        JSONArray msgs = new JSONArray();
        for (Message message : mMessages) {
            msgs.put(message.toJson());
        }

        conversation.put("messages", msgs);
        conversation.put("unreadMessageCount", mMessages.size());

        return conversation;
    }

    private boolean contains(String id) {
        return mMessages.stream().anyMatch(message -> message.getId().equals(id));
    }

    private class Message {
        String id;
        String text;
        String time;
        String phone;

        public Message(String id, String text, String time, String phone) {
            this.id = id;
            this.text = text;
            this.time = time;
            this.phone = phone;
        }

        public String getId() {
            return id;
        }

        public JSONObject toJson() throws JSONException {
            JSONObject payload = new JSONObject();
            payload.put("@type", "text");
            payload.put("text", text);

            JSONObject sender = new JSONObject();
            sender.put("address", phone);
            sender.put("addressType", "PhoneNumberAddress");

            JSONObject message = new JSONObject();
            message.put("id", id);
            message.put("messagePayload", payload);
            message.put("status", "unread");
            message.put("createdTime", time);
            message.put("sender", sender);

            return message;
        }
    }
}
