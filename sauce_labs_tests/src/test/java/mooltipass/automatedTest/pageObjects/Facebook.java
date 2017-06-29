package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Facebook extends AbstractPage{
		
		public Facebook (WebDriver driver) {
			super(driver);
			PageFactory.initElements(driver, this);
		}

		@FindBy(id = "email")
		private WebElement email;

		@FindBy(id = "pass")
		private WebElement password;
		
		@FindBy(id = "loginbutton")
		private WebElement submitLogin;
		
		
		@FindBy(id = "show_me_how_logout_1")
		private WebElement logoutBtn;
		
		@FindBy(id = "logoutMenu")
		private WebElement dashBoard;
		public void enterEmail(String value){
			email.sendKeys(value);
		}

		public void enterPassword(String value){
			waitUntilAppears(password);
			password.sendKeys(value);
		}
		
		
		
		public void submit(){
		submitLogin.click();
		}
		public void goTodDashboard()
		{
			waitUntilAppears(dashBoard);
			dashBoard.click();
			
		}
		public void logout(){

			sleep(3000);
			clickWithAction(logoutBtn);
		}
		
		public boolean checkLogin(){
			waitUntilAppears(By.id("logoutMenu"));
			return isElementPresent(By.id("logoutMenu"));
		}
		public boolean checkAtLoginPage(){
			return isElementPresent(By.id("email"));
		}
	
	
}
