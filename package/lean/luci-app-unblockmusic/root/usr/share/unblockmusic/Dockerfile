FROM node:lts-alpine

ENV NODE_ENV production

WORKDIR /usr/src/app
COPY package*.json ./
RUN npm install --production
COPY . .

EXPOSE 8080

ENTRYPOINT ["node", "app.js"]