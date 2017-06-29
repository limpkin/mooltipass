package mooltipass.automatedTest.pageObjects;

import org.openqa.selenium.By;
import org.openqa.selenium.WebDriver;
import org.openqa.selenium.WebElement;
import org.openqa.selenium.support.FindBy;
import org.openqa.selenium.support.PageFactory;

public class Techmania extends AbstractPage{

	public Techmania(WebDriver driver) {
		super(driver);
		// TODO Auto-generated constructor stub
		PageFactory.initElements(driver, this);
	}

	@FindBy(id = "txtLoginBox")
	private WebElement username;

	@FindBy(id = "txtPasswordBox")
	private WebElement password;

	@FindBy(id = "ibLogin")
	private WebElement loginBtn;
	
	@FindBy(xpath = "//a[text()='Logout']")
	private WebElement logoutBtn;
	
	@FindBy(xpath = "//a[@id='btnAccount']")
	private WebElement expandBtn;
	
	@FindBy(id = "loginLink")
	private WebElement loginLink;
	
	public void enterUsername(String value){
		waitUntilAppears(username);
		username.sendKeys(value);
	}
	
	public void enterPassword(String value){
		password.sendKeys(value);
	}
	
	public void submit(){
		loginBtn.click();
	}
	public void clickloginLink(){
		loginLink.click();
	}
	public boolean checkLogin(){
		//waitUntilAppears(By.xpath("//span[text()='moolti pass']"));
		return isElementPresent(By.xpath("//span[text()='moolti pass']"));
	}
	
	public void logout(){
		waitUntilAppears(expandBtn);
		expandBtn.click();
		sleep(500);
		logoutBtn.click();
	
	}
	
}
